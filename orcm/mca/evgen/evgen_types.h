/*
 * Copyright (c) 2009      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2012      Los Alamos National Security, Inc. All rights reserved.
 * Copyright (c) 2014-2015 Intel, Inc. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * @file:
 *
 */

#ifndef MCA_EVGEN_TYPES_H
#define MCA_EVGEN_TYPES_H

/*
 * includes
 */

#include "orcm_config.h"
#include "orcm/types.h"

#include "orcm/mca/mca.h"

BEGIN_C_DECLS

/* define RAS types */
typedef enum {
    ORCM_RAS_EXCEPTION,
    ORCM_RAS_TRANSITION,
    ORCM_RAS_SENSOR,
    ORCM_RAS_COUNTER
} orcm_ras_type_t;

/* define an event class */
typedef enum {
    ORCM_RAS_HARDWARE_EVENT,
    ORCM_RAS_SOFTWARE_EVENT,
    ORCM_RAS_ENVIRO_EVENT
} orcm_ras_class_t;

/* define RAS location object */
typedef struct {
    opal_object_t super;
    char *hostname;
    char *locstring;
} orcm_location_t;
OBJ_CLASS_DECLARATION(orcm_location_t);

/* define RAS severity flags */
typedef enum {
    ORCM_RAS_FATAL,
    ORCM_RAS_WARNING,
    ORCM_RAS_INFO
} orcm_ras_severity_t;

/* define a caddy for transporting RAS event info to the evgen framework */
typedef struct {
    opal_object_t super;
    opal_event_t ev;
    orcm_ras_class_t ras_class;
    orcm_ras_type_t type;
    orcm_location_t location;
    orcm_ras_severity_t severity;
    struct timeval timestamp;
} orcm_evgen_caddy_t;
OBJ_CLASS_DECLARATION(orcm_evgen_caddy_t);

END_C_DECLS

#endif /* MCA_EVGEN_TYPES_H */
