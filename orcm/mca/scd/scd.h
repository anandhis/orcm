/*
 * Copyright (c) 2014      Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef MCA_SCD_H
#define MCA_SCD_H

/*
 * includes
 */

#include "orcm_config.h"
#include "orcm/constants.h"

#include "opal/mca/mca.h"

#include "orte/runtime/orte_globals.h"
#include "orte/util/name_fns.h"
#include "orte/util/proc_info.h"

#include "orcm/mca/scd/scd_types.h"

BEGIN_C_DECLS

#define ORCM_ACTIVATE_SCHED_STATE(a, b)                                 \
    do {                                                                \
        opal_output_verbose(1, orcm_scd_base_framework.framework_output, \
                            "%s ACTIVATE SESSION %d STATE %s AT %s:%d",	\
                            ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),         \
                            (a)->id, orcm_session_state_to_str((b)),    \
                            __FILE__, __LINE__);			\
        orcm_sched_base_activate_session_state((a), (b));               \
    } while(0);

/*
 * Component functions - all MUST be provided!
 */

/* initialize the selected module */
typedef int (*orcm_scd_base_module_init_fn_t)(void);

/* finalize the selected module */
typedef void (*orcm_scd_base_module_finalize_fn_t)(void);

/* activate a session state */
typedef orcm_alloc_id_t (*orcm_scd_base_module_alloc_fn_t)(orcm_alloc_t *req);

/*
 * Ver 1.0
 */
struct orcm_scd_base_module_1_0_0_t {
    orcm_scd_base_module_init_fn_t        init;
    orcm_scd_base_module_finalize_fn_t    finalize;
    orcm_scd_base_module_alloc_fn_t       allocate;
};

typedef struct orcm_scd_base_module_1_0_0_t orcm_scd_base_module_1_0_0_t;
typedef orcm_scd_base_module_1_0_0_t orcm_scd_base_module_t;

/*
 * the standard component data structure
 */
struct orcm_scd_base_component_1_0_0_t {
    mca_base_component_t base_version;
    mca_base_component_data_t base_data;
};
typedef struct orcm_scd_base_component_1_0_0_t orcm_scd_base_component_1_0_0_t;
typedef orcm_scd_base_component_1_0_0_t orcm_scd_base_component_t;



/*
 * Macro for use in components that are of type scd v1.0.0
 */
#define ORCM_SCD_BASE_VERSION_1_0_0 \
  /* scd v1.0 is chained to MCA v2.0 */ \
  MCA_BASE_VERSION_2_0_0, \
  /* scd v1.0 */ \
  "scd", 1, 0, 0

/* Global structure for accessing name server functions
 */
ORCM_DECLSPEC extern orcm_scd_base_module_t orcm_sched;  /* holds selected module's function pointers */

END_C_DECLS

#endif
