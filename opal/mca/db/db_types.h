/*
 * Copyright (c) 2012-2013 Los Alamos National Security, Inc.  All rights reserved. 
 * Copyright (c) 2014      Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */
/** @file:
 *
 * The OPAL Database Framework
 *
 */

#ifndef OPAL_DB_TYPES_H
#define OPAL_DB_TYPES_H

#include "opal_config.h"
#include "opal/types.h"

#include "opal/dss/dss_types.h"

BEGIN_C_DECLS

/* some OPAL-appropriate key definitions */
#define OPAL_DB_LOCALITY    "opal.locality"
#define OPAL_DB_CPUSET      "opal.cpuset"
#define OPAL_DB_CREDENTIAL  "opal.cred"
#define OPAL_DB_JOB_SDIR    "opal.job.session.dir"
#define OPAL_DB_MY_SDIR     "opal.my.session.dir"
#define OPAL_DB_LOCALRANK   "opal.local.rank"
#define OPAL_DB_LOCALLDR    "opal.local.ldr"

END_C_DECLS

#endif
