dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2014      Intel, Inc. All rights reserved.
dnl $COPYRIGHT$
dnl 
dnl Additional copyrights may follow
dnl 
dnl $HEADER$
dnl

# MCA_sensor_pwr_CONFIG([action-if-found], [action-if-not-found])
# -----------------------------------------------------------
AC_DEFUN([MCA_orte_sensor_pwr_CONFIG], [
    AC_CONFIG_FILES([orte/mca/sensor/pwr/Makefile])

    AC_ARG_WITH([pwr],
                [AC_HELP_STRING([--with-pwr],
                                [Build pwr support (default: no)])],
	                        [], with_pwr=no)

    # do not build if support not requested
    AS_IF([test "$with_pwr" != "no"],
          [AS_IF([test "$opal_found_linux" = "yes"],
                 [$1],
                 [AC_MSG_WARN([Core power sensing was requested but is only supported on Intel-based Linux systems])
                  AC_MSG_ERROR([Cannot continue])
                  $2])],
          [$2])
])dnl
