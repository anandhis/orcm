/*
 * Copyright (c) 2014-2015 Intel, Inc.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "orcm_config.h"
#include "orcm/constants.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#include "opal/class/opal_list.h"
#include "opal/dss/dss.h"
#include "opal/util/argv.h"
#include "opal/util/if.h"
#include "opal/util/net.h"
#include "opal/util/opal_environ.h"
#include "opal/util/show_help.h"

#include "orte/runtime/orte_globals.h"
#include "orte/runtime/orte_wait.h"
#include "orte/util/name_fns.h"
#include "orte/util/show_help.h"
#include "orte/mca/errmgr/errmgr.h"

#include "orcm/mca/db/db.h"
#include "orcm/runtime/orcm_globals.h"
#include "orcm/util/utils.h"

#include "orcm/mca/evgen/base/base.h"
#include "orcm/mca/evgen/first/evgen_first.h"

/* API functions */

static void first_init(void);
static void first_finalize(void);
static void generate(orcm_evgen_caddy_t *cd);

/* The module struct */

orcm_evgen_base_module_t orcm_evgen_first_module = {
    first_init,
    first_finalize,
    generate
};

typedef struct {
    opal_object_t super;
    bool active;
    int status;
    int dbhandle;
    opal_list_t *list;
} first_caddy_t;
static void cdcon(first_caddy_t *p)
{
    p->list = NULL;
    p->active = true;
}
static void cddes(first_caddy_t *p)
{
    if (NULL != p->list) {
        OPAL_LIST_RELEASE(p->list);
    }
}
static OBJ_CLASS_INSTANCE(first_caddy_t,
                          opal_object_t,
                          cdcon, cddes);

/* local variables */
static bool got_handle = false;
static int dbhandle = -1;

static void cbfunc(int dbh, int status,
                   opal_list_t *in, opal_list_t *out,
                   void *cbdata)
{
    first_caddy_t *cd = (first_caddy_t*)cbdata;
    OPAL_OUTPUT_VERBOSE((1, orcm_evgen_base_framework.framework_output,
                         "%s evgen:first cbfunc for handle %d",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME), dbh));

    cd->status = status;
    cd->dbhandle = dbh;
    cd->active = false;
}

static void first_init(void)
{
    OPAL_OUTPUT_VERBOSE((1, orcm_evgen_base_framework.framework_output,
                         "%s evgen:first init",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME)));

    return;
}

static void first_finalize(void)
{
    first_caddy_t *cd;

    OPAL_OUTPUT_VERBOSE((1, orcm_evgen_base_framework.framework_output,
                         "%s evgen:first finalize",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME)));

    if (0 <= dbhandle) {
        /* setup a caddy */
        cd = OBJ_NEW(first_caddy_t);
        orcm_db.close(dbhandle, cbfunc, cd);
        ORTE_WAIT_FOR_COMPLETION(cd->active);
        dbhandle = -1;
        OBJ_RELEASE(cd);
    }
    return;
}

static void generate(orcm_evgen_caddy_t *ecd)
{
    first_caddy_t *cd;
    opal_value_t *kv;

    OPAL_OUTPUT_VERBOSE((1, orcm_evgen_base_framework.framework_output,
                         "%s evgen:first record event",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME)));

    if (!got_handle) {
        /* get a db handle */
        cd = OBJ_NEW(first_caddy_t);

        /* get a dbhandle for us */
        cd->list = OBJ_NEW(opal_list_t);
        kv = OBJ_NEW(opal_value_t);
        kv->key = strdup("components");
        kv->type = OPAL_STRING;
        kv->data.string = strdup("print");
        opal_list_append(cd->list, &kv->super);
        orcm_db.open("first", cd->list, cbfunc, cd);
        ORTE_WAIT_FOR_COMPLETION(cd->active);
        if (ORCM_SUCCESS == cd->status && 0 <= cd->dbhandle) {
            dbhandle = cd->dbhandle;
            got_handle = true;
        }
        OBJ_RELEASE(cd);
    }
    cd = OBJ_NEW(first_caddy_t);
    cd->list = OBJ_NEW(opal_list_t);

    kv = OBJ_NEW(opal_value_t);
    kv->key = strdup("TYPE");
    kv->type = OPAL_STRING;
    kv->data.string = strdup(orcm_evgen_base_print_type(ecd->type));
    opal_list_append(cd->list, &kv->super);

    kv = OBJ_NEW(opal_value_t);
    kv->key = strdup("CLASS");
    kv->type = OPAL_STRING;
    kv->data.string = strdup(orcm_evgen_base_print_class(ecd->ras_class));
    opal_list_append(cd->list, &kv->super);

    kv = OBJ_NEW(opal_value_t);
    kv->key = strdup("SEVERITY");
    kv->type = OPAL_STRING;
    kv->data.string = strdup(orcm_evgen_base_print_severity(ecd->severity));
    opal_list_append(cd->list, &kv->super);

    orcm_db.store(dbhandle, "RAS-EVENT", cd->list, cbfunc, cd);
    ORTE_WAIT_FOR_COMPLETION(cd->active);
    OBJ_RELEASE(cd);

    return;
}
