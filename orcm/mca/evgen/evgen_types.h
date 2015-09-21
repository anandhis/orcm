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
    ORCM_RAS_COUNTER,
    /* add a marker for the end of the list so
     * automated tools can better handle it */
    ORCM_RAS_TYPE_MAX
} orcm_ras_type_t;

/* define an event class */
typedef enum {
    ORCM_RAS_HARDWARE_EVENT,
    ORCM_RAS_SOFTWARE_EVENT,
    ORCM_RAS_ENVIRO_EVENT,
    /* add a marker for the end of the list so
     * automated tools can better handle it */
    ORCM_RAS_CLASS_MAX

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
    ORCM_RAS_INFO,
    /* add a marker for the end of the list so
     * automated tools can better handle it */
    ORCM_RAS_SEVERITY_MAX

} orcm_ras_severity_t;

/* define the RAS events */
typedef enum {
    /* chip-level events */
    ORCM_RAS_EVENT_CORE_TEMP_HI,
    ORCM_RAS_EVENT_CORE_TEMP_LO,
    ORCM_RAS_EVENT_CPU_FREQ_HI,
    ORCM_RAS_EVENT_CPU_FREQ_LO,
    ORCM_RAS_EVENT_CPU_FREQ_UNSETTABLE,
    ORCM_RAS_EVENT_GOVERNOR_UNSETTABLE,
    /* node-level events */
    ORCM_RAS_EVENT_NODE_POWER_HI,
    ORCM_RAS_EVENT_NODE_POWER_LO,
    /* process-level events */
    ORCM_RAS_EVENT_MEMORY_USE_HI,
    ORCM_RAS_EVENT_CPU_USE_HI,
    ORCM_RAS_EVENT_NETWORK_USE_HI,
    ORCM_RAS_EVENT_DISK_USE_HI,
    /* VDU events */
    ORCM_RAS_EVENT_COOLANT_FLOW_LO,
    ORCM_RAS_EVENT_COOLANT_PRESSURE_LO,
    ORCM_RAS_EVENT_COOLANT_INLET_TEMP_HI,
    ORCM_RAS_EVENT_COOLANT_INLET_TEMP_LO,
    ORCM_RAS_EVENT_VDU_POWER_HI,
    ORCM_RAS_EVENT_VDU_POWER_LO,
    /* add a marker for the end of the list so
     * automated tools can better handle it */
    ORCM_RAS_EVENT_MAX

} orcm_ras_event_t;

/* define a caddy for transporting RAS event info to the evgen framework */
typedef struct {
    opal_object_t super;
    opal_event_t ev;
    orcm_ras_event_t id;
    orcm_ras_class_t ras_class;
    orcm_ras_type_t type;
    orcm_location_t location;
    orcm_ras_severity_t severity;
    struct timeval timestamp;
} orcm_evgen_caddy_t;
OBJ_CLASS_DECLARATION(orcm_evgen_caddy_t);

END_C_DECLS

#endif /* MCA_EVGEN_TYPES_H */
