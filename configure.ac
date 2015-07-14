AC_PREREQ([2.65])

AC_INIT([xoreos-tools],[0.0.0],[xoreos-devel@xoreos.org],[xoreos-tools],[https://xoreos.org/])

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

dnl Compiler warning flags
AC_ARG_WITH([warnings], [AS_HELP_STRING([--without-warnings], [Compile without extra warnings @<:@default=no@:>@])], [], [with_warnings=yes])

if test "x$with_warnings" = "xno"; then
	WARN_C_CXX=""
	WARN_CXX=""
else
	WARN_C_CXX="-Wall"
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wignored-qualifiers])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wpointer-arith])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wshadow])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wsign-compare])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wtype-limits])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wuninitialized])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wunused-parameter])
	AX_CHECK_COMPILER_FLAGS_VAR([C], [WARN_C_CXX], [-Wunused-but-set-parameter])

	WARN_CXX=""
	AX_CHECK_COMPILER_FLAGS_VAR([C++], [WARN_CXX], [-Wnon-virtual-dtor])
	AX_CHECK_COMPILER_FLAGS_VAR([C++], [WARN_CXX], [-Wno-overloaded-virtual])
fi

AC_SUBST(WARN_C_CXX)
AC_SUBST(WARN_CXX)

dnl Link-time optimization
AC_ARG_WITH([lto], [AS_HELP_STRING([--with-lto], [Compile with link-time optimization @<:@default=no@:>@])], [], [with_lto=no])

LTO=""
if test "x$with_lto" = "xyes"; then
	AX_CHECK_COMPILER_FLAGS_VAR([C], [LTO], [-flto])
fi

AC_SUBST(LTO)

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

dnl String to integer/float conversion functions
AC_CHECK_FUNCS([strtoll])
AC_CHECK_FUNCS([strtoull])
AC_CHECK_FUNCS([strtof])

dnl General purpose libraries
AX_CHECK_ICONV( , AC_MSG_ERROR([No useable iconv() function found!]))
AX_CHECK_ZLIB(1, 2, 3, 4, , AC_MSG_ERROR([zlib(>= 1.2.3.4) is required and could not be found!]))

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
AC_CONFIG_FILES([src/images/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
