AC_INIT(1.h)
AC_CONFIG_AUX_DIR(admin)

IOL_MAJOR_VERSION=0
IOL_MINOR_VERSION=0
IOL_MICRO_VERSION=2
IOL_VERSION=$IOL_MAJOR_VERSION.$IOL_MINOR_VERSION.$IOL_MICRO_VERSION

VERSION=$IOL_VERSION
PACKAGE=IOLsucker

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local)

AC_ARG_ENABLE(debug,
  [  --enable-debug        creates debugging code],
  [  IOLsucker_use_debug="$enableval"],[openmine_use_debug="no"])

  CFLAGS=`echo $CFLAGS | sed s/-g//`
  if test "$IOLsucker_use_debug" = "yes"; then
        CFLAGS="$CFLAGS -g"
  fi


# Build time sanity check...
AM_SANITY_CHECK

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_MINGW32

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_ISC_POSIX

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([stdlib.h string.h ])
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AM_PATH_CURL
AM_PATH_XML2
AC_CHECK_FUNCS([memset strchr strtol])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strtol])
AC_FUNC_STAT

CFLAGS="$CFLAGS -Wall -W -ansi -Winline -Wno-unused -I ../lib"
CXXFLAGS=$CFLAGS

dnl --------------------------------------------------------------------------
dnl Optionnal stuff
dnl --------------------------------------------------------------------------

LIBS="$LIBS "

dnl --------------------------------------------------------------------------
dnl Substitutions
dnl --------------------------------------------------------------------------
AC_DEFINE_UNQUOTED(UNIX,1, [a plataform])
AC_DEFINE_UNQUOTED(WINDOWS,2, [a plataform])
AC_DEFINE_UNQUOTED(SYSTEM, UNIX, [2nd dlopen parameter])

AC_SUBST(PACKAGE)
AC_SUBST(IOL_VERSION)


dnl --------------------------------------------------------------------------
dnl Output
dnl --------------------------------------------------------------------------
AC_OUTPUT([
Makefile
lib/Makefile
src/Makefile],
[echo ])


echo
echo "IOLsucker configure report:"
echo
echo "     CFLAGS                        : $CFLAGS"
echo "     LIBS                          : $LIBS"
echo "     Locales in use                : $ALL_LINGUAS"
echo "==============================================================="
echo
echo "Type \"make\" to compile IOLsucker."
echo

