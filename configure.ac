AC_INIT([iolsucker],[2.0.16])
AC_CONFIG_AUX_DIR(admin)

IOL_MAJOR_VERSION=2
IOL_MINOR_VERSION=0
IOL_MICRO_VERSION=16
IOL_VERSION=$IOL_MAJOR_VERSION.$IOL_MINOR_VERSION.$IOL_MICRO_VERSION
DATE="1 Mayo 2003"

VERSION=$IOL_VERSION
PACKAGE=iolsucker

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local)

AC_ARG_ENABLE(debug,
  [  --enable-debug        creates debugging code],
  [  IOLsucker_use_debug="$enableval"],[IOLsucker_use_debug="no"])

  CFLAGS=`echo $CFLAGS | sed s/-g//`
  if test "$IOLsucker_use_debug" = "yes"; then
        CFLAGS="$CFLAGS -g"
  fi


# Build time sanity check...
AM_SANITY_CHECK

# Checks for programs.
AC_PROG_CC
AC_MINGW32

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_ISC_POSIX
AC_PATH_PROG([iol_have_groff],[groff],[yes],[no])
AC_PATH_PROG([iol_have_ps2pdf],[ps2pdf],[yes],[no])


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([stdlib.h string.h limits.h])
AC_CHECK_HEADERS([syslog.h unistd.h])
AC_CHECK_HEADERS([sys/time.h sys/file.h])
AC_CHECK_HEADERS([termios.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_SIGNAL
AC_TYPE_PID_T
AC_STRUCT_TM

# Checks for library functions.
AM_PATH_CURL(,,AC_MSG_ERROR([$PACKAGE requiere de libcurl]))
AM_CURL_WORKING_VERSION
AC_TRY_RUN([
	#include <curl/curl.h>
	int
	main()
	{
		CURLOPT_DEBUGDATA;
		return 0;
	}
],,[AC_DEFINE(HAVE_DEBUGDATA,[],[libcurl has CURLOPT_DEBUGDATA])])

AM_PATH_XML2(,,AC_MSG_ERROR([$PACKAGE requiere de libxml2]))
AM_PATH_GLIB(,,AC_MSG_ERROR([$PACKAGE requiere la GLIB]))
AM_PATH_GTK(,[iolsucker_gtk="yes"],[iolscuker_gtk="no"])

AC_CHECK_FUNCS([memset strchr strtol strrchr tcgetattr tcsetattr])
AC_CHECK_FUNCS([strchr strdup strerror strchr strstr mkdir gettimeofday])
AC_CHECK_FUNCS([strtol isatty])
AC_CHECK_FUNCS([memchr])
AC_FUNC_STAT
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_FUNC_FORK

CFLAGS="$CFLAGS -Wall  -I ../lib"

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

if [[  $iolsucker_gtk == "yes" ]] ; then
    IOLWIZARD="iolwizard"
else
    IOLWIZARD=""
fi

if [[ $iol_have_groff == "yes" ]] ; then
    IOL_HTML_PAGES="iolsucker.html"
else
    IOL_HTML_PAGES=""
fi


IOL_PDF_PAGES=""
if [[ ${iol_have_ps2pdf} == "yes" ]] ; then
    if [[  $iol_have_groff == "yes" ]] ; then
        IOL_PDF_PAGES="iolsucker.pdf"
    else
        iol_have_ps2pdf="no"
    fi
else
    iol_have_ps2pdf="no"
fi

AC_SUBST(PACKAGE)
AC_SUBST(IOL_VERSION)
AC_SUBST(IOLWIZARD)
AC_SUBST(IOL_HTML_PAGES)
AC_SUBST(IOL_PDF_PAGES)
AC_SUBST(DATE)
dnl --------------------------------------------------------------------------
dnl Output
dnl --------------------------------------------------------------------------
AC_OUTPUT([
Makefile
bin/Makefile
doc/Makefile
doc/iolsucker.1
lib/Makefile
src/Makefile
win32/Makefile
win32/libmisc/Makefile
win32/iolsucker/Makefile
win32/iolwizard/Makefile
win32/iolunch/Makefile
extra/Makefile
extra/inst/Makefile
iolunch/Makefile
iolwizard/Makefile])


echo     "Programs to compile:"
echo    "   iolsucker......yes"
echo    "   iolwizard......${iolsucker_gtk}" 
echo    "   html doc.......${iol_have_groff}" 
echo    "   pdf doc........${iol_have_ps2pdf}" 
echo 
echo "Type \"make\" to compile IOLsucker."
echo
