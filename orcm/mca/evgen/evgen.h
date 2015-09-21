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

#ifndef MCA_EVGEN_H
#define MCA_EVGEN_H

/*
 * includes
 */

#include "orcm_config.h"
#include "orcm/types.h"

#include "orcm/mca/mca.h"

#include "orte/types.h"

#include "orcm/mca/evgen/evgen_types.h"

BEGIN_C_DECLS

/*
 * Component functions - all MUST be provided!
 */

/* initialize the module */
typedef void (*orcm_evgen_module_init_fn_t)(void);

/* finalize the module */
typedef void (*orcm_evgen_module_fini_fn_t)(void);

/* generate a RAS event in the module's format - the module
 * is allowed to ignore the event if the incoming alert
 * is not supported in this module */
typedef void (*orcm_evgen_module_generate_fn_t)(orcm_evgen_caddy_t *cd);

/*
 * Ver 1.0
 */
struct orcm_evgen_base_module_1_0_0_t {
    orcm_evgen_module_init_fn_t      init;
    orcm_evgen_module_fini_fn_t      finalize;
    orcm_evgen_module_generate_fn_t  generate;
};

typedef struct orcm_evgen_base_module_1_0_0_t orcm_evgen_base_module_1_0_0_t;
typedef orcm_evgen_base_module_1_0_0_t orcm_evgen_base_module_t;

/*
 * the standard component data structure
 */
struct orcm_evgen_base_component_1_0_0_t {
    mca_base_component_t base_version;
    mca_base_component_data_t base_data;
    char *data_measured;
};
typedef struct orcm_evgen_base_component_1_0_0_t orcm_evgen_base_component_1_0_0_t;
typedef orcm_evgen_base_component_1_0_0_t orcm_evgen_base_component_t;

/*
 * Macro for use in components that are of type evgen v1.0.0
 */
#define ORCM_EVGEN_BASE_VERSION_1_0_0 \
  /* evgen v1.0 is chained to MCA v2.0 */ \
    ORCM_MCA_BASE_VERSION_2_1_0("evgen", 1, 0, 0)


END_C_DECLS

#endif /* MCA_EVGEN_H */
