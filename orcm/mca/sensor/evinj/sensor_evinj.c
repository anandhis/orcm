/*
 * Copyright (c) 2009-2011 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2011-2012 Los Alamos National Security, LLC.
 *                         All rights reserved.
 * Copyright (c) 2014-2015 Intel, Inc.  All rights reserved.
 *
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
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <ctype.h>

#include "opal_stdint.h"
#include "opal/util/alfg.h"
#include "opal/util/output.h"

#include "orte/mca/errmgr/errmgr.h"
#include "orte/util/name_fns.h"
#include "orte/runtime/orte_globals.h"

#include "orcm/mca/evgen/base/base.h"
#include "orcm/mca/sensor/base/base.h"
#include "orcm/mca/sensor/base/sensor_private.h"
#include "sensor_evinj.h"

/* declare the API functions */
static int init(void);
static void finalize(void);
static void sample(orcm_sensor_sampler_t *sampler);

/* instantiate the module */
orcm_sensor_base_module_t orcm_sensor_evinj_module = {
    init,
    finalize,
    NULL,
    NULL,
    sample,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

FILE *fp = NULL;

static char *orcm_getline(void)
{
    char *ret, *buff;
    char input[1024];
    int k;

    ret = fgets(input, 1024, fp);
    if (NULL != ret) {
        /* trim the end of the line */
        for (k=strlen(input)-1; 0 < k && isspace(input[k]); k--) {
            input[k] = '\0';
        }
        buff = strdup(input);
        return buff;
    }

    return NULL;
}

static int init(void)
{
    /* if we were given one, open the vector file */
    if (NULL != mca_sensor_evinj_component.vector_file) {
        fp = fopen(mca_sensor_evinj_component.vector_file, "r");
    }

    return ORCM_SUCCESS;
}

static void finalize(void)
{
    /* close the vector file */
    if (NULL != fp) {
        fclose(fp);
    }
}

static void sample(orcm_sensor_sampler_t *sampler)
{
    float prob;
    char *vector, **elements;
    orcm_ras_type_t type;
    orcm_ras_class_t class;
    orcm_ras_severity_t severity;

    OPAL_OUTPUT_VERBOSE((1, orcm_sensor_base_framework.framework_output,
                         "%s sample:evinj considering injecting something",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME)));

     /* roll the dice */
    prob = (double)opal_rand(&mca_sensor_evinj_component.rng_buff) / (double)UINT32_MAX;
    if (prob < mca_sensor_evinj_component.prob) {
        /* if we were given a vector file, read
         * the next vector from the file */
        if (NULL != fp) {
            vector = orcm_getline();
            if (NULL == vector) {
                /* reopen the file to start over */
                fclose(fp);
                fp = fopen(mca_sensor_evinj_component.vector_file, "r");
                if (NULL == fp) {
                    /* nothing we can do */
                    return;
                }
                vector = orcm_getline();
                if (NULL == vector) {
                    /* give up */
                    return;
                }
            }
            elements = opal_argv_split(vector, ':');
            free(vector);
            if (0 == strcmp("EXCEPTION", elements[0])) {
                type = ORCM_RAS_EXCEPTION;
            } else if (0 == strcmp("TRANSITION", elements[0])) {
                type = ORCM_RAS_TRANSITION;
            } else if (0 == strcmp("SENSOR", elements[0])) {
                type = ORCM_RAS_SENSOR;
            } else if (0 == strcmp("COUNTER", elements[0])) {
                type = ORCM_RAS_COUNTER;
            } else {
                ORTE_ERROR_LOG(ORCM_ERR_BAD_PARAM);
                opal_argv_free(elements);
                return;
            }
            if (0 == strcmp("HARDWARE", elements[1])) {
                class = ORCM_RAS_HARDWARE_EVENT;
            } else if (0 == strcmp("SOFTWARE", elements[1])) {
                class = ORCM_RAS_SOFTWARE_EVENT;
            } else if (0 == strcmp("ENVIRO", elements[1])) {
                class = ORCM_RAS_ENVIRO_EVENT;
            } else {
                ORTE_ERROR_LOG(ORCM_ERR_BAD_PARAM);
                opal_argv_free(elements);
                return;
            }
            if (0 == strcmp("FATAL", elements[2])) {
                severity = ORCM_RAS_FATAL;
            } else if (0 == strcmp("WARNING", elements[2])) {
                severity = ORCM_RAS_WARNING;
            } else if (0 == strcmp("INFO", elements[2])) {
                severity = ORCM_RAS_INFO;
            } else {
                ORTE_ERROR_LOG(ORCM_ERR_BAD_PARAM);
                opal_argv_free(elements);
                return;
            }
            opal_argv_free(elements);
        } else {
            /* randomly generate a vector */
            prob = (double)opal_rand(&mca_sensor_evinj_component.rng_buff) / (double)UINT32_MAX;
            if (0.75 < prob) {
                type = ORCM_RAS_EXCEPTION;
            } else if (0.5 < prob) {
                type = ORCM_RAS_TRANSITION;
            } else if (0.25 < prob) {
                type = ORCM_RAS_SENSOR;
            } else {
                type = ORCM_RAS_COUNTER;
            }
            prob = (double)opal_rand(&mca_sensor_evinj_component.rng_buff) / (double)UINT32_MAX;
            if (0.67 < prob) {
                class = ORCM_RAS_HARDWARE_EVENT;
            } else if (0.33 < prob) {
                class = ORCM_RAS_SOFTWARE_EVENT;
            } else {
                class = ORCM_RAS_ENVIRO_EVENT;
            }
            prob = (double)opal_rand(&mca_sensor_evinj_component.rng_buff) / (double)UINT32_MAX;
            if (0.67 < prob) {
                severity = ORCM_RAS_FATAL;
            } else if (0.33 < prob) {
                severity = ORCM_RAS_WARNING;
            } else {
                severity = ORCM_RAS_INFO;
            }
        }
        opal_output_verbose(1, orcm_sensor_base_framework.framework_output,
                             "%s sample:evinj injecting RAS event",
                             ORTE_NAME_PRINT(ORTE_PROC_MY_NAME));

        /* inject it into the event generator thread */
        ORCM_RAS_EVENT(type, class, severity);
    }
}
