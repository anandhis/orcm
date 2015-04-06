/*
 * Copyright (c) 2013-2015 Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "orcm_config.h"
#include "orcm/constants.h"
#include "orcm/types.h"

#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif  /* HAVE_STRING_H */
#include <stdio.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif  /* HAVE_DIRENT_H */
#include <ctype.h>

#include "opal_stdint.h"
#include "opal/class/opal_list.h"
#include "opal/dss/dss.h"
#include "opal/util/os_path.h"
#include "opal/util/output.h"
#include "opal/util/os_dirpath.h"

#include "orte/util/name_fns.h"
#include "orte/util/show_help.h"
#include "orte/runtime/orte_globals.h"
#include "orte/mca/errmgr/errmgr.h"
#include "orte/mca/notifier/notifier.h"
#include "orte/mca/notifier/base/base.h"

#include "orcm/mca/db/db.h"

#include "orcm/mca/sensor/base/base.h"
#include "orcm/mca/sensor/base/sensor_private.h"
#include "sensor_mcedata.h"

/* declare the API functions */
static int init(void);
static void finalize(void);
static void start(orte_jobid_t job);
static void stop(orte_jobid_t job);
static void mcedata_sample(orcm_sensor_sampler_t *sampler);
static void mcedata_log(opal_buffer_t *buf);

/* instantiate the module */
orcm_sensor_base_module_t orcm_sensor_coretemp_module = {
    init,
    finalize,
    start,
    stop,
    mcedata_sample,
    mcedata_log,
    NULL,
    NULL
};

typedef struct {
    opal_list_item_t super;
    char *file;
    int socket;
    int core;
    char *label;
    float critical_temp;
    float max_temp;
} mcedata_tracker_t;

static void ctr_con(mcedata_tracker_t *trk)
{
    trk->file = NULL;
    trk->label = NULL;
    trk->socket = -1;
    trk->core = -1;
}
static void ctr_des(mcedata_tracker_t *trk)
{
    if (NULL != trk->file) {
        free(trk->file);
    }
    if (NULL != trk->label) {
        free(trk->label);
    }
}
OBJ_CLASS_INSTANCE(mcedata_tracker_t,
                   opal_list_item_t,
                   ctr_con, ctr_des);

static bool log_enabled = true;
static opal_list_t tracking;

static char *orte_getline(FILE *fp)
{
    char *ret, *buff;
    char input[1024];

    ret = fgets(input, 1024, fp);
    if (NULL != ret) {
	   input[strlen(input)-1] = '\0';  /* remove newline */
	   buff = strdup(input);
	   return buff;
    }
    
    return NULL;
}

/* FOR FUTURE: extend to read cooling device speeds in
 *     current speed: /sys/class/thermal/cooling_deviceN/cur_state
 *     max speed: /sys/class/thermal/cooling_deviceN/max_state
 *     type: /sys/class/thermal/cooling_deviceN/type
 */
static int init(void)
{
    DIR *cur_dirp = NULL, *tdir;
    struct dirent *dir_entry, *entry;
    char *dirname = NULL;
    char *filename, *ptr, *tmp;
    size_t tlen = strlen("temp");
    size_t ilen = strlen("_input");
    FILE *fp;
    mcedata_tracker_t *trk, *t2;
    bool inserted;
    opal_list_t foobar;
    int corecount = 0;
    char *skt;

    /* always construct this so we don't segfault in finalize */
    OBJ_CONSTRUCT(&tracking, opal_list_t);

    /*
     * Open up the base directory so we can get a listing
     */
    if (NULL == (cur_dirp = opendir("/sys/bus/platform/devices"))) {
        OBJ_DESTRUCT(&tracking);
        orte_show_help("help-orcm-sensor-mcedata.txt", "req-dir-not-found",
                       true, orte_process_info.nodename,
                       "/sys/bus/platform/devices");
        return ORTE_ERROR;
    }

    /*
     * For each directory
     */
    while (NULL != (dir_entry = readdir(cur_dirp))) {
        
        /* look for coretemp directories */
        if (0 != strncmp(dir_entry->d_name, "coretemp", strlen("coretemp"))) {
            continue;
        }
        skt = strchr(dir_entry->d_name, '.');
        if (NULL != skt) {
            skt++;
        }

        /* open that directory */
        if (NULL == (dirname = opal_os_path(false, "/sys/bus/platform/devices", dir_entry->d_name, NULL ))) {
            continue;
        }
        if (NULL == (tdir = opendir(dirname))) {
            free(dirname);
            continue;
        }
        OBJ_CONSTRUCT(&foobar, opal_list_t);
        while (NULL != (entry = readdir(tdir))) {
            /*
             * Skip the obvious
             */
            if (NULL == entry->d_name) {
                continue;
            }

            if (0 == strncmp(entry->d_name, ".", strlen(".")) ||
                0 == strncmp(entry->d_name, "..", strlen(".."))) {
                continue;
            }
            if (strlen(entry->d_name) < (tlen+ilen)) {
                /* cannot be a core temp file */
                continue;
            }
            /*
             * See if this is a core temp file
             */
            if (0 != strncmp(entry->d_name, "temp", strlen("temp"))) {
                continue;
            }
            if (0 != strcmp(entry->d_name + strlen(entry->d_name) - ilen, "_input")) {
                continue;
            }
            /* track the info for this core */
            trk = OBJ_NEW(mcedata_tracker_t);
            if (NULL != skt) {
                trk->socket = strtol(skt, NULL, 10);
            }
            trk->file = opal_os_path(false, dirname, entry->d_name, NULL);
            /* take the part up to the first underscore as this will
             * be used as the start of all the related files
             */
            tmp = strdup(entry->d_name);
            if (NULL == (ptr = strchr(tmp, '_'))) {
                /* unrecognized format */
                free(tmp);
                OBJ_RELEASE(trk);
                continue;
            }
            *ptr = '\0';
            /* look for critical, max, and label info */
            asprintf(&filename, "%s/%s_%s", dirname, tmp, "label");
            if (NULL != (fp = fopen(filename, "r")))
            {
                if (NULL != (trk->label = orte_getline(fp)))
                {
                    fclose(fp);
                    free(filename);
                    if(NULL != (ptr = strcasestr(trk->label,"core"))) {
                        trk->core = strtol(trk->label+strlen("core "), NULL, 10); /* This stores the core ID under each processor*/
                    } else if (NULL != (ptr = strcasestr(trk->label,"Physical id "))) {
                    } else {
                        free(tmp);
                        OBJ_RELEASE(trk);
                        continue; /* We only collect core temperatures for now*/
                    }
                } else {
                    ORTE_ERROR_LOG(ORTE_ERR_FILE_READ_FAILURE);
                    fclose(fp);
                    free(filename);
                    free(tmp);
                    OBJ_RELEASE(trk);
                    continue;
                }
            } else {
                ORTE_ERROR_LOG(ORTE_ERR_FILE_OPEN_FAILURE);
                free(filename);
                free(tmp);
                OBJ_RELEASE(trk);
                    continue;
            }
            asprintf(&filename, "%s/%s_%s", dirname, tmp, "crit");
            if (NULL != (fp = fopen(filename, "r")))
            {
                if (NULL != (ptr = orte_getline(fp)))
                {
                    trk->critical_temp = strtol(ptr, NULL, 10)/1000.0;
                    free(ptr);
                    fclose(fp);
                    free(filename);
                } else {
                    ORTE_ERROR_LOG(ORTE_ERR_FILE_READ_FAILURE);
                    fclose(fp);
                    free(filename);
                    free(tmp);
                    OBJ_RELEASE(trk);
                    continue;
                }
            } else {
                ORTE_ERROR_LOG(ORTE_ERR_FILE_OPEN_FAILURE);
                free(filename);
                free(tmp);
                OBJ_RELEASE(trk);
                continue;
            }

            asprintf(&filename, "%s/%s_%s", dirname, tmp, "max");
            if (NULL != (fp = fopen(filename, "r")))
            {
                if (NULL != (ptr = orte_getline(fp)))
                {
                    trk->max_temp = strtol(ptr, NULL, 10)/1000.0;
                    free(ptr);
                    fclose(fp);
                    free(filename);
                } else {
                    ORTE_ERROR_LOG(ORTE_ERR_FILE_READ_FAILURE);
                    fclose(fp);
                    free(filename);
                    free(tmp);
                    OBJ_RELEASE(trk);
                    continue;
                }
            } else {
                ORTE_ERROR_LOG(ORTE_ERR_FILE_OPEN_FAILURE);
                free(filename);
                OBJ_RELEASE(trk);
                free(tmp);
                continue;
            }

            /* add to our list, in core order */
            inserted = false;
            OPAL_LIST_FOREACH(t2, &foobar, mcedata_tracker_t) {
                if (NULL != strcasestr(trk->label,"core")) {
                    if (t2->core > trk->core) {
                        opal_list_insert_pos(&foobar, &t2->super, &trk->super);
                        inserted = true;
                        break;
                    }
                }
            }
            if (!inserted) {
                opal_list_append(&foobar, &trk->super);
            }
            /* cleanup */
            free(tmp);
        }
        free(dirname);
        closedir(tdir);
        /* add the ordered list to our collection */
        while (NULL != (t2 = (mcedata_tracker_t*)opal_list_remove_first(&foobar))) {
            if(NULL != strcasestr(t2->label,"core")) {
                t2->core = corecount++;
                sprintf(t2->label, "core %d", t2->core);
            }
            opal_list_append(&tracking, &t2->super);
        }
        OPAL_LIST_DESTRUCT(&foobar);
    }
    closedir(cur_dirp);

    if (0 == opal_list_get_size(&tracking)) {
        /* nothing to read */
        orte_show_help("help-orcm-sensor-coretemp.txt", "no-cores-found",
                       true, orte_process_info.nodename);
        return ORTE_ERROR;
    }

    return ORCM_SUCCESS;
}

static void finalize(void)
{
    OPAL_LIST_DESTRUCT(&tracking);
}

/*
 * Start monitoring of local temps
 */
static void start(orte_jobid_t jobid)
{
    return;
}


static void stop(orte_jobid_t jobid)
{
    return;
}

static void mcedata_sample(orcm_sensor_sampler_t *sampler)
{
    int ret;
    char *temp;
    opal_buffer_t data, *bptr;
    time_t now;
    char time_str[40];
    char *timestamp_str;
    bool packed;
    struct tm *sample_time;

    if (0 == opal_list_get_size(&tracking)) {
        return;
    }

    /* prep to store the results */
    OBJ_CONSTRUCT(&data, opal_buffer_t);
    packed = false;

    /* pack our name */
    temp = strdup("mcedata");
    if (OPAL_SUCCESS != (ret = opal_dss.pack(&data, &temp, 1, OPAL_STRING))) {
        ORTE_ERROR_LOG(ret);
        OBJ_DESTRUCT(&data);
        return;
    }
    free(temp);

    /* store our hostname */
    if (OPAL_SUCCESS != (ret = opal_dss.pack(&data, &orte_process_info.nodename, 1, OPAL_STRING))) {
        ORTE_ERROR_LOG(ret);
        OBJ_DESTRUCT(&data);
        return;
    }

    /* get the sample time */
    now = time(NULL);
    /* pass the time along as a simple string */
    sample_time = localtime(&now);
    if (NULL == sample_time) {
        ORTE_ERROR_LOG(OPAL_ERR_BAD_PARAM);
        return;
    }
    strftime(time_str, sizeof(time_str), "%F %T%z", sample_time);
    asprintf(&timestamp_str, "%s", time_str);
    if (OPAL_SUCCESS != (ret = opal_dss.pack(&data, &timestamp_str, 1, OPAL_STRING))) {
        ORTE_ERROR_LOG(ret);
        OBJ_DESTRUCT(&data);
        free(timestamp_str);
        return;
    }
    free(timestamp_str);

    /* xfer the data for transmission */
    if (packed) {
        bptr = &data;
        if (OPAL_SUCCESS != (ret = opal_dss.pack(&sampler->bucket, &bptr, 1, OPAL_BUFFER))) {
            ORTE_ERROR_LOG(ret);
            OBJ_DESTRUCT(&data);
            return;
        }
    }
    OBJ_DESTRUCT(&data);
}

static void mycleanup(int dbhandle, int status,
                      opal_list_t *kvs, void *cbdata)
{
    OPAL_LIST_RELEASE(kvs);
    if (ORTE_SUCCESS != status) {
        log_enabled = false;
    }
}

static void mcedata_log(opal_buffer_t *sample)
{
    char *hostname=NULL;
    char *sampletime;
    int rc;
    int32_t n;
    opal_list_t *vals;
    opal_value_t *kv;

    if (!log_enabled) {
        return;
    }

    /* unpack the host this came from */
    n=1;
    if (OPAL_SUCCESS != (rc = opal_dss.unpack(sample, &hostname, &n, OPAL_STRING))) {
        ORTE_ERROR_LOG(rc);
        return;
    }

    /* sample time */
    n=1;
    if (OPAL_SUCCESS != (rc = opal_dss.unpack(sample, &sampletime, &n, OPAL_STRING))) {
        ORTE_ERROR_LOG(rc);
        return;
    }

    opal_output_verbose(3, orcm_sensor_base_framework.framework_output,
                        "%s Received log from host %s with xx cores",
                        ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),
                        (NULL == hostname) ? "NULL" : hostname);

    /* xfr to storage */
    vals = OBJ_NEW(opal_list_t);

    /* load the sample time at the start */
    if (NULL == sampletime) {
        ORTE_ERROR_LOG(OPAL_ERR_BAD_PARAM);
        return;
    }
    kv = OBJ_NEW(opal_value_t);
    kv->key = strdup("ctime");
    kv->type = OPAL_STRING;
    kv->data.string = strdup(sampletime);
    free(sampletime);
    opal_list_append(vals, &kv->super);

    /* load the hostname */
    if (NULL == hostname) {
        ORTE_ERROR_LOG(OPAL_ERR_BAD_PARAM);
        goto cleanup;
        return;
    }
    kv = OBJ_NEW(opal_value_t);
    kv->key = strdup("hostname");
    kv->type = OPAL_STRING;
    kv->data.string = strdup(hostname);
    opal_list_append(vals, &kv->super);

    /* store it */
    if (0 <= orcm_sensor_base.dbhandle) {
        orcm_db.store(orcm_sensor_base.dbhandle, "mcedata", vals, mycleanup, NULL);
    } else {
        OPAL_LIST_RELEASE(vals);
    }

 cleanup:
    if (NULL != hostname) {
        free(hostname);
    }

}
