/*
 * Copyright (c) 2013-2014 Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/** @file:
 */

#ifndef MCA_SCHED_BASE_H
#define MCA_SCHED_BASE_H

/*
 * includes
 */
#include "orcm_config.h"

#include "opal/mca/mca.h"
#include "opal/mca/event/event.h"
#include "opal/dss/dss_types.h"

#include "orcm/mca/scd/scd.h"


BEGIN_C_DECLS

/*
 * MCA framework
 */
ORCM_DECLSPEC extern mca_base_framework_t orcm_scd_base_framework;
/*
 * Select an available component.
 */
ORCM_DECLSPEC int orcm_scd_base_select(void);

typedef struct {
    /* flag that we just want to test */
    bool test_mode;
    /* define an event base strictly for scheduling - this
     * allows the scheduler to respond to requests for
     * information without interference with the
     * actual scheduling computation
     */
    opal_event_base_t *ev_base;
    bool ev_active;
    /* state machine */
    opal_list_t states;
    /* queues for pending session requests */
    opal_list_t queues;
} orcm_scd_base_t;
ORCM_DECLSPEC extern orcm_scd_base_t orcm_scd_base;

/* start/stop base receive */
ORCM_DECLSPEC int orcm_scd_base_comm_start(void);
ORCM_DECLSPEC int orcm_scd_base_comm_stop(void);

/* base code stubs */
ORCM_DECLSPEC void orcm_sched_base_activate_session_state(orcm_session_t *s,
                                                          orcm_session_state_t state);

/* datatype support */
ORCM_DECLSPEC int orcm_pack_alloc(opal_buffer_t *buffer, const void *src,
                                  int32_t num_vals, opal_data_type_t type);
ORCM_DECLSPEC int orcm_unpack_alloc(opal_buffer_t *buffer, void *dest,
                                    int32_t *num_vals, opal_data_type_t type);
ORCM_DECLSPEC int orcm_compare_alloc(orcm_alloc_t *value1,
                                     orcm_alloc_t *value2,
                                     opal_data_type_t type);
ORCM_DECLSPEC int orcm_copy_alloc(orcm_alloc_t **dest,
                                  orcm_alloc_t *src,
                                  opal_data_type_t type);
ORCM_DECLSPEC int orcm_print_alloc(char **output, char *prefix,
                                   orcm_alloc_t *src, opal_data_type_t type);

ORCM_DECLSPEC const char *orcm_session_state_to_str(orcm_session_state_t state);
ORCM_DECLSPEC int orcm_sched_base_add_session_state(orcm_session_state_t state,
                                                    orcm_state_cbfunc_t cbfunc,
                                                    int priority);


END_C_DECLS
#endif
