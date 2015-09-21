/*
 * Copyright (c) 2009      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2013      Los Alamos National Security, LLC.  All rights reserved.
 * Copyright (c) 2014-2015 Intel, Inc.  All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 */

#ifndef MCA_EVGEN_BASE_H
#define MCA_EVGEN_BASE_H

/*
 * includes
 */
#include "orcm_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#include "opal/class/opal_list.h"
#include "opal/mca/base/base.h"
#include "opal/mca/event/event.h"

#include "orcm/runtime/orcm_globals.h"
#include "orcm/mca/evgen/evgen.h"

BEGIN_C_DECLS

/* define a macro to be used for declaring a RAS event */
#define ORCM_RAS_EVENT(a, b, c)                                              \
    do {                                                                     \
        orcm_evgen_caddy_t *_cd;                                             \
        opal_output_verbose(1, orcm_evgen_base_framework.framework_output,   \
                            "%s RAS EVENT: %s:%d",                           \
                            ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),              \
                            __FILE__, __LINE__);                             \
        _cd = OBJ_NEW(orcm_evgen_caddy_t);                                   \
        _cd->type = (a);                                                     \
        _cd->ras_class = (b);                                                \
        _cd->severity = (c);                                                 \
        opal_event_set(orcm_evgen_base.evbase, &(_cd->ev), -1,               \
                       OPAL_EV_WRITE,                                        \
                       orcm_evgen_base_event, _cd);                          \
        opal_event_active((&_cd->ev), OPAL_EV_WRITE, 1);                     \
    } while(0);

/*
 * MCA Framework
 */

typedef struct {
    opal_event_base_t *evbase;
    opal_list_t actives;
} orcm_evgen_base_t;

typedef struct {
    opal_list_item_t super;
    int priority;
    mca_base_component_t *component;
    orcm_evgen_base_module_t *module;
} orcm_evgen_active_module_t;
OBJ_CLASS_DECLARATION(orcm_evgen_active_module_t);

ORCM_DECLSPEC int orcm_evgen_base_select(void);

ORCM_DECLSPEC extern mca_base_framework_t orcm_evgen_base_framework;
ORCM_DECLSPEC extern orcm_evgen_base_t orcm_evgen_base;

/* stub functions */
ORCM_DECLSPEC void orcm_evgen_base_event(int sd, short args, void *cbdata);
ORCM_DECLSPEC const char* orcm_evgen_base_print_type(orcm_ras_type_t t);
ORCM_DECLSPEC const char* orcm_evgen_base_print_class(orcm_ras_class_t c);
ORCM_DECLSPEC const char* orcm_evgen_base_print_severity(orcm_ras_severity_t s);

END_C_DECLS
#endif
