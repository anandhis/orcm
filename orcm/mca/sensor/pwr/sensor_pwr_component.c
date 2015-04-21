/*
 * Copyright (c) 2013-2015 Intel, Inc. All rights reserved.
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "orcm_config.h"
#include "orcm/constants.h"

#include "opal/mca/base/base.h"
#include "opal/mca/base/mca_base_var.h"

#include "orcm/mca/sensor/base/base.h"
#include "sensor_pwr.h"

/*
 * Local functions
 */

static int orcm_sensor_pwr_open(void);
static int orcm_sensor_pwr_close(void);
static int orcm_sensor_pwr_query(mca_base_module_t **module, int *priority);
static int pwr_component_register(void);

orcm_sensor_pwr_component_t mca_sensor_pwr_component = {
    {
        {
            ORCM_SENSOR_BASE_VERSION_1_0_0,
            /* Component name and version */
            .mca_component_name = "pwr",
            MCA_BASE_MAKE_VERSION(component, ORCM_MAJOR_VERSION, ORCM_MINOR_VERSION,
                                  ORCM_RELEASE_VERSION),
        
            /* Component open and close functions */
            .mca_open_component = orcm_sensor_pwr_open,
            .mca_close_component = orcm_sensor_pwr_close,
            .mca_query_component = orcm_sensor_pwr_query,
            .mca_register_component_params = pwr_component_register
        },
        .base_data = {
            /* The component is checkpoint ready */
            MCA_BASE_METADATA_PARAM_CHECKPOINT
        },
        "pwr"  // data being sensed
    }
};

/**
  * component open/close/init function
  */
static int orcm_sensor_pwr_open(void)
{
    return ORCM_SUCCESS;
}

static int orcm_sensor_pwr_query(mca_base_module_t **module, int *priority)
{
    *priority = 50;  /* ahead of heartbeat */
    *module = (mca_base_module_t *)&orcm_sensor_pwr_module;
    return ORCM_SUCCESS;
}

/**
 *  Close all subsystems.
 */

static int orcm_sensor_pwr_close(void)
{
    return ORCM_SUCCESS;
}

static int pwr_component_register(void)
{
    mca_base_component_t *c = &mca_sensor_pwr_component.super.base_version;

     mca_sensor_pwr_component.test = false;
    (void) mca_base_component_var_register (c, "test",
                                            "Generate and pass test vector",
                                            MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                            OPAL_INFO_LVL_9,
                                            MCA_BASE_VAR_SCOPE_READONLY,
                                            & mca_sensor_pwr_component.test);
    return ORCM_SUCCESS;
}
