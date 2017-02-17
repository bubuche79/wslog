AC_PREREQ([2.69])
AC_INIT([wslog], [0.7], [bubuche.pub@free.fr])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

AM_PROG_AR
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

# Checks for programs.
LT_INIT
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([check], [srunner_create],	[HAVE_LIBCHECK=1])
AC_CHECK_LIB([curl], [curl_global_init], [ ])
AC_CHECK_LIB([sqlite3], [sqlite3_initialize], [ ])

AM_CONDITIONAL([USE_LIBCHECK], [test "x$HAVE_LIBCHECK" = "x1"])

# Add WS23XX device support
AC_ARG_ENABLE([ws23xx],
	[AS_HELP_STRING([--disable-ws23xx], [Disable La Crosse Technology WS23XX support])],
	[], [enable_ws23xx=yes])

AS_IF([test "x$enable_ws23xx" != "xno"],
	[AC_DEFINE(HAVE_WS23XX, 1, [La Crosse Technology WS23XX support])])

AM_CONDITIONAL([USE_WS23XX], [test "x$enable_ws23xx" = "xyes"])

# Add simulator device support
AC_ARG_ENABLE([simu],
	[AS_HELP_STRING([--enable-simu], [Enable simulator device support])],
	[], [enable_simu=no])

AS_IF([test "x$enable_simu" != "xno"],
	[AC_DEFINE(HAVE_SIMU, 1, [Simulator device support])])

AM_CONDITIONAL([USE_SIMU], [test "x$enable_simu" = "xyes"])

# Add Weather Station View CGI device support
AC_ARG_ENABLE([wsview],
	[AS_HELP_STRING([--disable-wsview], [Disable Weather Station View])],
	[], [AC_CHECK_HEADER(lua.h, [enable_wsview=yes])])

AS_IF([test "x$enable_wsview" != "xno"],
	[AC_DEFINE(HAVE_WSVIEW, 1, [Weather Station View])])

AM_CONDITIONAL([USE_WSVIEW], [test "x$enable_ws23xx" = "xyes"])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/libws/Makefile
                 src/ws23xx/Makefile
                 src/wslogd/Makefile
                 tests/Makefile
                 ])
AC_OUTPUT
