/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2010      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2012-2013 Los Alamos National Security, Inc. All rights reserved.
 * Copyright (c) 2014-2015 Intel, Inc. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */


#include "orcm_config.h"
#include "orcm/constants.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "opal/mca/mca.h"
#include "opal/util/argv.h"
#include "opal/util/fd.h"
#include "opal/util/output.h"
#include "opal/mca/base/base.h"
#include "opal/class/opal_pointer_array.h"
#include "opal/runtime/opal_progress_threads.h"

#include "orte/mca/errmgr/errmgr.h"
#include "orte/mca/rml/rml.h"
#include "orte/util/regex.h"

#include "orcm/runtime/orcm_globals.h"
#include "orcm/mca/cfgi/base/base.h"
#include "orcm/mca/evgen/base/base.h"

/*
 * The following file was created by configure.  It contains extern
 * statements and the definition of an array of pointers to each
 * component's public mca_base_component_t struct.
 */

#include "orcm/mca/evgen/base/static-components.h"

/*
 * Global variables
 */
orcm_evgen_base_t orcm_evgen_base;

static int orcm_evgen_base_close(void)
{
    if (NULL != orcm_evgen_base.evbase) {
        opal_progress_thread_finalize("evgen");
    }

    OPAL_LIST_DESTRUCT(&orcm_evgen_base.actives);

    /* Close all remaining available components */
    return mca_base_framework_components_close(&orcm_evgen_base_framework, NULL);
}

/**
 * Function for finding and opening either all MCA components, or the one
 * that was specifically requested via a MCA parameter.
 */
static int orcm_evgen_base_open(mca_base_open_flag_t flags)
{
    int rc;

    /* construct the array of active modules */
    OBJ_CONSTRUCT(&orcm_evgen_base.actives, opal_list_t);

    /* start the progress thread */
      if (NULL == (orcm_evgen_base.evbase = opal_progress_thread_init("evgen"))) {
        return ORCM_ERROR;
    }

    /* Open up all available components */
    if (OPAL_SUCCESS != (rc = mca_base_framework_components_open(&orcm_evgen_base_framework, flags))) {
        return rc;
    }

    return OPAL_SUCCESS;
}

MCA_BASE_FRAMEWORK_DECLARE(orcm, evgen, "ORCM Event generation", NULL,
                           orcm_evgen_base_open, orcm_evgen_base_close,
                           mca_evgen_base_static_components, 0);


const char* orcm_evgen_base_print_type(orcm_ras_type_t t)
{
    switch(t) {
        case ORCM_RAS_EXCEPTION:
            return "EXCEPTION";
        case ORCM_RAS_TRANSITION:
            return "TRANSITION";
        case ORCM_RAS_SENSOR:
            return "SENSOR";
        case ORCM_RAS_COUNTER:
            return "COUNTER";
        default:
            return "UNKNOWN";
    }
}

const char* orcm_evgen_base_print_class(orcm_ras_class_t c)
{
    switch(c) {
        case ORCM_RAS_HARDWARE_EVENT:
            return "HARDWARE";
        case ORCM_RAS_SOFTWARE_EVENT:
            return "SOFTWARE";
        case ORCM_RAS_ENVIRO_EVENT:
            return "ENVIRO";
        default:
            return "UNKNOWN";
    }
}

const char* orcm_evgen_base_print_severity(orcm_ras_severity_t s)
{
    switch(s) {
        case ORCM_RAS_FATAL:
            return "FATAL";
        case ORCM_RAS_WARNING:
            return "WARNING";
        case ORCM_RAS_INFO:
            return "INFO";
        default:
            return "UNKNOWN";
    }
}

/* framework class instanstiations */
OBJ_CLASS_INSTANCE(orcm_evgen_active_module_t,
                   opal_list_item_t,
                   NULL, NULL);

static void locon(orcm_location_t *p)
{
    p->hostname = NULL;
    p->locstring = NULL;
}
static void lodes(orcm_location_t *p)
{
    if (NULL != p->hostname) {
        free(p->hostname);
    }
    if (NULL != p->locstring) {
        free(p->locstring);
    }
}
OBJ_CLASS_INSTANCE(orcm_location_t,
                   opal_object_t,
                   locon, lodes);

OBJ_CLASS_INSTANCE(orcm_evgen_caddy_t,
                   opal_object_t,
                   NULL, NULL);
