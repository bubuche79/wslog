AC_PREREQ([2.69])
LT_PREREQ([2.4.6])
AC_INIT([wslog], [0.8.4], [bubuche.pub@free.fr])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

AM_PROG_AR
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

# Checks for programs.
LT_INIT([disable-static])
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([check], [srunner_create])
AC_CHECK_LIB([curl], [curl_global_init])
AC_CHECK_LIB([sqlite3], [sqlite3_initialize])

AM_CONDITIONAL([USE_LIBCHECK], [test "x$ac_cv_lib_check_srunner_create" = "xyes"])

# Add Davis Vantage device support
AC_ARG_ENABLE([vantage],
	[AS_HELP_STRING([--disable-vantage], [Disable Davis Vantage support])],
	[], [enable_vantage=yes])

AS_IF([test "x$enable_vantage" != "xno"],
	[AC_DEFINE(HAVE_VANTAGE, 1, [Davis Vantage support])])

AM_CONDITIONAL([USE_VANTAGE], [test "x$enable_vantage" = "xyes"])

# Add WS23XX device support
AC_ARG_ENABLE([ws23xx],
	[AS_HELP_STRING([--disable-ws23xx], [Disable La Crosse Technology WS23XX support])],
	[], [enable_ws23xx=yes])

AS_IF([test "x$enable_ws23xx" != "xno"],
	[AC_DEFINE(HAVE_WS23XX, 1, [La Crosse Technology WS23XX support])])

AM_CONDITIONAL([USE_WS23XX], [test "x$enable_ws23xx" = "xyes"])

# Add virtual device support
AC_ARG_ENABLE([virt],
	[AS_HELP_STRING([--enable-virt], [Enable virtual device support])],
	[], [enable_virt=no])

AS_IF([test "x$enable_virt" != "xno"],
	[AC_DEFINE(HAVE_VIRT, 1, [Virtual device support])])

AM_CONDITIONAL([USE_VIRT], [test "x$enable_virt" = "xyes"])

# Add Weather Station View CGI device support
AC_ARG_ENABLE([wsview],
	[AS_HELP_STRING([--disable-wsview], [Disable Weather Station View])],
	[], [AC_CHECK_HEADER(lua.h, [enable_wsview=yes])])

AS_IF([test "x$enable_wsview" != "xno"],
	[AC_DEFINE(HAVE_WSVIEW, 1, [Weather Station View])])

AM_CONDITIONAL([USE_WSVIEW], [test "x$enable_wsview" = "xyes"])

AC_CONFIG_FILES([Makefile
                 docs/Makefile
                 src/Makefile
                 src/libws/Makefile
                 src/vantage/Makefile
                 src/ws23xx/Makefile
                 src/wslogd/Makefile
                 src/wsview/Makefile
                 tests/Makefile
                 ])
AC_OUTPUT
