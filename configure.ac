AC_INIT([tascamgtk], [0.4], [onkel@paraair.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PREREQ([2.66])
AC_PROG_CXX

AC_ARG_ENABLE([osc], 
    AS_HELP_STRING([--enable-osc], [Enable OSC server, default: yes]),
        [case "${enableval}" in
                     yes) osc=yes ;;
                     no)  osc=no ;;
                     *)   AC_MSG_ERROR([bad value ${enableval} for --enable-osc]) ;;
        esac],
        [osc=yes])

AS_IF([test "x$osc" != xno],
	[
        PKG_CHECK_MODULES([LO], [liblo])
	AM_CONDITIONAL(HAVE_OSC, true)
	AC_DEFINE(HAVE_OSC,[1],[Define to 1 if you have the `OSC server' function.])
	], [
	AM_CONDITIONAL(HAVE_OSC, false)
	])


PKG_CHECK_MODULES([GTKMM3], [gtkmm-3.0])
PKG_CHECK_MODULES([ALSA], [alsa])
PKG_CHECK_MODULES([XML2], [libxml++-3.0])

GLIB_GSETTINGS

AC_CHECK_HEADER_STDBOOL

AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strstr])
AC_TYPE_INT32_T
AC_TYPE_SIZE_T

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

