AC_INIT([tascamgtk], [0.1], [onkel@paraair.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PREREQ([2.66])
AC_PROG_CC
AC_PROG_CXX

# PKG_CHECK_MODULES([GTK2], [gtk+-2.0])
PKG_CHECK_MODULES([GTKMM2], [gtkmm-2.4])
PKG_CHECK_MODULES([ALSA], [alsa])


AC_CHECK_HEADER_STDBOOL

AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])


AC_OUTPUT
