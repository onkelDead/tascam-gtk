AC_INIT([tascamgtk], [0.5], [onkel@paraair.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PREREQ([2.66])

AC_ARG_ENABLE([debug], 
    AS_HELP_STRING([--enable-debug], [Enable debugging, default: no]),
        [case "${enableval}" in
                     yes) CXXFLAGS='-g3 -O0' ;;
                     no)   ;;
                     *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
        esac],
        [])

AC_PROG_CXX


AC_ARG_ENABLE([osc], 
    AS_HELP_STRING([--enable-osc], [Enable OSC server, default: yes]),
        [case "${enableval}" in
                     yes) osc=yes ;;
                     no)  osc=no ;;
                     *)   AC_MSG_ERROR([bad value ${enableval} for --enable-osc]) ;;
        esac],
        [osc=yes])

AC_ARG_ENABLE([xml3], 
    AS_HELP_STRING([--enable-xml3], [Enable libxml++-3, default: no]),
        [case "${enableval}" in
                    yes) xml=yes ;;
                    no) xml=no ;;
                    *) AC_MSG_ERROR([bad value ${enableval} for --enable-xml3]) ;;
        esac],
        [xml=auto])

AS_IF([test "x$osc" != xno],
	[
        PKG_CHECK_MODULES([LO], [liblo])
	AM_CONDITIONAL(HAVE_OSC, true)
	AC_DEFINE(HAVE_OSC,[1],[Define to 1 if you have the `OSC server' function.])
	], [
	AM_CONDITIONAL(HAVE_OSC, false)
	])

AS_CASE("x$xml",
        [xyes],[
            PKG_CHECK_MODULES([XML3], [libxml++-3.0], [AC_DEFINE([HAVE_XML3], [1], [Use XML3])], [AC_MSG_ERROR([missing library libxml++-3.0])])
        ],
        [xno],[
            PKG_CHECK_MODULES([XML26], [libxml++-2.6])
            AC_DEFINE([HAVE_XML26], [1], [Use XML26])
        ],
        [PKG_CHECK_MODULES([XML3], [libxml++-3.0],
                      [AC_DEFINE([HAVE_XML3], [1], [Use XML3])],
                      [PKG_CHECK_MODULES([XML26], [libxml++-2.6],
                                         [AC_DEFINE([HAVE_XML26], [1], [Use XML26])
                                         ])
                      ])
        ]
    )



PKG_CHECK_MODULES([GTKMM3], [gtkmm-3.0])
PKG_CHECK_MODULES([ALSA], [alsa])
PKG_CHECK_MODULES([XML3], [libxml++-5.0],
				  [AC_DEFINE([HAVE_XML3], [1], [Use XML3])],
				  [PKG_CHECK_MODULES([XML3], [libxml++-4.0],
									[AC_DEFINE([HAVE_XML3], [1], [Use XML3])],
									[PKG_CHECK_MODULES([XML3], [libxml++-3.0],
													  [AC_DEFINE([HAVE_XML3], [1], [Use XML3])],
													  [PKG_CHECK_MODULES([XML26], [libxml++-2.6],
																		 [AC_DEFINE([HAVE_XML26], [1], [Use XML26])
																		 ])
													  ])
									])
				  ])

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

