#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(playmusic,1.0,wjlkoorey@hotmail.com)
AC_CONFIG_SRCDIR([playmusic.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(playmusic,1.0)
# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([asound],[snd_pcm_open],
	[LIBS="-lasound $LIBS"
	CFLAGS="-DHAVE_ALSA $CFLAGS"
	])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
