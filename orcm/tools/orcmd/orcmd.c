/*
 * Copyright (c) 2013      Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "orcm_config.h"
#include "orcm/constants.h"

#include <stdio.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "opal/util/cmd_line.h"
#include "opal/util/opal_environ.h"
#include "opal/util/show_help.h"
#include "opal/mca/base/mca_base_var.h"
#include "opal/mca/event/event.h"

#include "orte/util/proc_info.h"
#include "orte/runtime/orte_globals.h"
#include "orte/mca/errmgr/errmgr.h"

#include "orcm/runtime/runtime.h"

static struct {
    bool help;
    bool version;
    bool verbose;
    bool master;
} orcm_globals;

static opal_cmd_line_init_t cmd_line_init[] = {
    /* Various "obvious" options */
    { NULL, 'h', NULL, "help", 0,
      &orcm_globals.help, OPAL_CMD_LINE_TYPE_BOOL,
      "This help message" },

    { NULL, 'V', NULL, "version", 0,
      &orcm_globals.version, OPAL_CMD_LINE_TYPE_BOOL,
      "Print version and exit" },

    { NULL, 'v', NULL, "verbose", 0,
      &orcm_globals.verbose, OPAL_CMD_LINE_TYPE_BOOL,
      "Be verbose" },

    { "cfgi_base_config_file", 's', "site-file", "site-file", 1,
      NULL, OPAL_CMD_LINE_TYPE_STRING,
      "Site configuration file for this orcm chain" },

    { "cfgi_base_validate", '\0', "validate-config", "validate-config", 0,
      NULL, OPAL_CMD_LINE_TYPE_BOOL,
      "Validate site file and exit" },

    /* End of list */
    { NULL, '\0', NULL, NULL, 0,
      NULL, OPAL_CMD_LINE_TYPE_NULL, NULL }
};

int main(int argc, char *argv[])
{
    int ret;
    opal_cmd_line_t cmd_line;
    char *ctmp;
    time_t now;

    /* process the cmd line arguments to get any MCA params on them */
    opal_cmd_line_create(&cmd_line, cmd_line_init);
    mca_base_cmd_line_setup(&cmd_line);
    if (ORCM_SUCCESS != (ret = opal_cmd_line_parse(&cmd_line, false, argc, argv)) ||
        orcm_globals.help) {
        char *args = NULL;
        args = opal_cmd_line_get_usage_msg(&cmd_line);
        fprintf(stderr, "Usage: %s [OPTION]...\n%s\n", argv[0], args);
        free(args);
        return ret;
    }
    
    if (orcm_globals.version) {
        fprintf(stderr, "orcm %s\n", ORCM_VERSION);
        exit(0);
    }

    /*
     * Since this process can now handle MCA/GMCA parameters, make sure to
     * process them.
     */
    mca_base_cmd_line_process_args(&cmd_line, &environ, &environ);

    /***************
     * Initialize
     ***************/
    if (ORCM_SUCCESS != (ret = orcm_init(ORCM_DAEMON))) {
        return ret;
    }

    /* print out a message alerting that we are alive */
    now = time(NULL);
    /* convert the time to a simple string */
    ctmp = ctime(&now);
    /* strip the trailing newline */
    ctmp[strlen(ctmp)-1] = '\0';

    if (ORCM_PROC_IS_AGGREGATOR) {
        opal_output(0, "%s: ORCM aggregator %s started",
                    ctmp, ORTE_NAME_PRINT(ORTE_PROC_MY_NAME));
    } else {
        opal_output(0, "%s: ORCM daemon %s started and connected to aggregator %s",
                    ctmp, ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),
                    ORTE_NAME_PRINT(ORTE_PROC_MY_DAEMON));
    }
    while (orte_event_base_active) {
        opal_event_loop(orte_event_base, OPAL_EVLOOP_ONCE);
    }

    /* print out a message alerting that we are shutting down */
    now = time(NULL);
    /* convert the time to a simple string */
    ctmp = ctime(&now);
    /* strip the trailing newline */
    ctmp[strlen(ctmp)-1] = '\0';

    if (ORCM_PROC_IS_AGGREGATOR) {
        opal_output(0, "%s: ORCM aggregator %s shutting down with status %d (%s)",
                    ctmp, ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),
                    orte_exit_status,
                    (0 < orte_exit_status) ? "SYS" : ORTE_ERROR_NAME(orte_exit_status));
    } else {
        opal_output(0, "%s: ORCM daemon %s shutting down with status %d (%s)",
                    ctmp, ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),
                    orte_exit_status,
                    (0 < orte_exit_status) ? "SYS" : ORTE_ERROR_NAME(orte_exit_status));
    }
    
     /***************
     * Cleanup
     ***************/
    orcm_finalize();

    return ret;
}
