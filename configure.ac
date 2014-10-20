AC_PREREQ([2.65])

AC_INIT([xoreos-tools],[0.0.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.11 -Wall -Wno-portability no-dist-gzip dist-xz subdir-objects])
# Support silent build rules. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
AM_SILENT_RULES([yes])

dnl If AM_PROG_AR exists, call it, to shut up a libtool warning (and make it more portable, I guess?)
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl libtool
LT_PREREQ([2.2])
LT_INIT([disable-shared])

dnl We're C++
AC_PROG_CXX
AM_PROG_CC_C_O
dnl We want a make install
AC_PROG_INSTALL

dnl --with-werror
AC_ARG_WITH([werror], [AS_HELP_STRING([--with-werror], [Compile with -Werror @<:@default=no@:>@])], [], [with_werror=no])
if test "x$with_werror" = "xyes"; then
	WERROR="-Werror -Werror=unused-but-set-variable"
fi

dnl Standard C, C++
AC_C_CONST
AC_HEADER_STDC

dnl Endianness
AC_C_BIGENDIAN()

dnl Special variables of the size of pointers
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T

dnl math library and special float functions
AC_CHECK_LIB([m], [cos])
AC_CHECK_FUNCS([fabs])
AC_CHECK_FUNCS([fmin])
AC_CHECK_FUNCS([fmax])
AC_CHECK_FUNCS([fabsf])
AC_CHECK_FUNCS([fminf])
AC_CHECK_FUNCS([fmaxf])

dnl General purpose libraries
AX_CHECK_ICONV( , AC_MSG_ERROR([No useable iconv() function found!]))

dnl Extra flags
case "$target" in
	*darwin*)
		XOREOSTOOLS_CFLAGS="-DUNIX -DMACOSX"
		XOREOSTOOLS_LIBS=""
		;;
	*mingw*)
		XOREOSTOOLS_CFLAGS="-mconsole"
		XOREOSTOOLS_LIBS=""
		;;
	*)
		XOREOSTOOLS_CFLAGS="-DUNIX"
		XOREOSTOOLS_LIBS=""
		;;
esac;

AC_SUBST(XOREOSTOOLS_CFLAGS)
AC_SUBST(XOREOSTOOLS_LIBS)

AC_SUBST(WERROR)

AC_CONFIG_FILES([utf8cpp/Makefile])
AC_CONFIG_FILES([src/common/Makefile])
AC_CONFIG_FILES([src/aurora/Makefile])
AC_CONFIG_FILES([src/xml/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
