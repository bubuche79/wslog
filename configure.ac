AC_PREREQ([2.69])
LT_PREREQ([2.4.6])
AC_INIT([wslog], [0.8.4], [gae.buchetet@free.fr])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

AM_PROG_AR
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

# Checks for programs.
LT_INIT([disable-static])
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES([LIBCURL], [libcurl])
PKG_CHECK_MODULES([SQLITE3], [sqlite3])
PKG_CHECK_MODULES([CHECK], [check],, [with_check=no])
PKG_CHECK_MODULES([LUA], [lua-5.1],, [with_lua=no])

AM_CONDITIONAL([USE_LIBCHECK], [test "x$with_check" != "xno"])

# Add Davis Vantage device support
AC_ARG_ENABLE([vantage],
	AS_HELP_STRING([--disable-vantage], [Disable Davis Vantage support]),,
	[enable_vantage=yes])

AS_IF([test "x$enable_vantage" != "xno"], [
	AC_DEFINE(HAVE_VANTAGE, 1, [Davis Vantage support])
])

AM_CONDITIONAL([USE_VANTAGE], [test "x$enable_vantage" != "xno"])

# Add WS23XX device support
AC_ARG_ENABLE([ws23xx],
	AS_HELP_STRING([--enable-ws23xx], [Enable La Crosse Technology WS23XX support]),,
	[enable_ws23xx=no])

AS_IF([test "x$enable_ws23xx" = "xyes"], [
	AC_DEFINE(HAVE_WS23XX, 1, [La Crosse Technology WS23XX support])
])

AM_CONDITIONAL([USE_WS23XX], [test "x$enable_ws23xx" = "xyes"])

# Add virtual device support
AC_ARG_ENABLE([virt],
	[AS_HELP_STRING([--enable-virt], [Enable virtual device support])],,
	[enable_virt=no])

AS_IF([test "x$enable_virt" = "xyes"],
	AC_DEFINE(HAVE_VIRT, 1, [Virtual device support])
])

AM_CONDITIONAL([USE_VIRT], [test "x$enable_virt" = "xyes"])

# Add Weather Station View CGI device support
AC_ARG_ENABLE([wsview],
	AS_HELP_STRING([--disable-wsview], [Disable Weather Station View]),,
	[enable_wsview=yes])

AS_IF([test "x$enable_wsview" != "xno"], [
	AS_IF([test "x$with_lua" != "xno"],, [enable_wsview=no])

	AS_IF([test "x$enable_wsview" != "xno"], [
		AC_DEFINE(HAVE_WSVIEW, 1, [Weather Station View])
	])
])

AM_CONDITIONAL([USE_WSVIEW], [test "x$enable_wsview" = "xyes"])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/libws/Makefile
	src/vantage/Makefile
	src/ws23xx/Makefile
	src/wslogd/Makefile
	src/wsview/Makefile
	tests/Makefile
])

AC_CONFIG_FILES(systemd/system/wslogd.service)

AC_OUTPUT
