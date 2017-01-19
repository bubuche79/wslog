#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([wslog], [0.6], [bubuche.pub@free.fr])
AC_CONFIG_HEADERS([config.h])

AM_PROG_AR
LT_INIT
#AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([check], [srunner_create], [ ])
AC_CHECK_LIB([curl], [curl_global_init], [ ])
AC_CHECK_LIB([m], [pow, log], [ ])
AC_CHECK_LIB([rt], [timer_settime], [ ])
AC_CHECK_LIB([pthread], [pthread_create], [ ])
AC_CHECK_LIB([sqlite3], [sqlite3_initialize], [ ])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h locale.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h syslog.h termio.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime dup2 ftruncate localtime_r memset munmap pow select setenv setlocale stpcpy strchr strdup strerror strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/lib/Makefile
                 src/lib/libws/Makefile
                 src/cmd/Makefile
                 src/cmd/ws23xx/Makefile
                 src/cmd/wslogd/Makefile
                 ])
AC_OUTPUT
