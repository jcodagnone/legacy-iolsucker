dnl AM_PATH_CURL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libcurl, and define CURL_CFLAGS and CURL_LIBS
dnl
AC_DEFUN(AM_PATH_CURL,
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(curl,[  --with-curl=PFX   Prefix where libcurl is installed
(optional)], curl_prefix="$withval", curl_prefix="")
AC_ARG_WITH(curl-libraries,[  --with-curl-libraries=DIR   Directory where
libcurl library is installed (optional)], curl_libraries="$withval",
curl_libraries="")
AC_ARG_WITH(curl-includes,[  --with-curl-includes=DIR   Directory where
libcurl header files are installed (optional)], curl_includes="$withval",
curl_includes="")
AC_ARG_ENABLE(curltest, [  --disable-curltest       Do not try to compile and
run a test libcurl program],, enable_curltest=yes)

if test "x$curl_prefix" != "xno" ; then

  if test "x$curl_libraries" != "x" ; then
    CURL_LIBS="-L$curl_libraries"
  elif test "x$curl_prefix" != "x" ; then
    CURL_LIBS="-L$curl_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    CURL_LIBS="-L$prefix/lib"
  fi

  CURL_LIBS="$CURL_LIBS -lcurl"

  if test "x$curl_includes" != "x" ; then
    CURL_CFLAGS="-I$curl_includes"
  elif test "x$curl_prefix" != "x" ; then
    CURL_CFLAGS="-I$curl_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    CURL_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for libcurl)
  no_curl=""


  if test "x$enable_curltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $CURL_CFLAGS"
    LIBS="$LIBS $CURL_LIBS"
dnl
dnl Now check if the installed libcurl is sufficiently new.
dnl
      rm -f conf.curltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

int main ()
{
  system("touch conf.curltest");
  return 0;
}

],, no_curl=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_curl" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])    
  else
     AC_MSG_RESULT(no)
     if test -f conf.curltest ; then
       :
     else
       echo "*** Could not run libcurl test program, checking why..."
       CFLAGS="$CFLAGS $CURL_CFLAGS"
       LIBS="$LIBS $CURL_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <curl/curl.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually
means"
       echo "*** that the run-time linker is not finding libcurl or finding
the wrong"
       echo "*** version of libcurl. If it is not finding libcurl, you'll need
to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf
to point"
       echo "*** to the installed location  Also, make sure you have run
ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove
it, although"
       echo "*** you may also be able to get things to work by modifying
LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file
config.log for the"
       echo "*** exact error that occured. This usually means libcurl was
incorrectly installed"
       echo "*** or that you have moved libcurl since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     CURL_CFLAGS=""
     CURL_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_DEFINE(HAVE_CURL, 1, [Define if you have libcurl.])
else
  CURL_CFLAGS=""
  CURL_LIBS=""
fi
  AC_SUBST(CURL_CFLAGS)
  AC_SUBST(CURL_LIBS)
  rm -f conf.curltest
])

AC_DEFUN(AM_CURL_WORKING_VERSION,
[ 
	AC_MSG_CHECKING(for a sane libcurl)
	l=`curl-config --version|cut -d' ' -f2`
	MAJOR="`echo $l | cut -d. -f1`"
	MINOR="`echo $l | cut -d. -f2`"
	AC_MSG_RESULT($l)

	if [[ "$l" ==  "7.10.3" ]] ; then
		exit -1
	fi
	AC_MSG_CHECKING(for a CURLOPT_PROXYTYPE in libcurl)
	l=""
	test $MAJOR -ge 7 &&  test $MINOR -ge 10 && AC_DEFINE([HAVE_PROXYTYPE],[],[curl option]) 

	AC_MSG_RESULT($l)

	AC_MSG_CHECKING(for a CURLOPT_DEBUGDATA in libcurl)
AC_TRY_RUN([
	#include <curl/curl.h>
	int
	main()
	{
		CURLOPT_DEBUGDATA;
		return 0;
	}],[l=yes],[l=no])
	AC_MSG_RESULT($l)
	if [[ $l == "yes" ]]; then
		AC_DEFINE(HAVE_CURLOPT_DEBUGDATA,[],[libcurl has CURLOPT_DEBUGDATA])
	fi

	AC_MSG_CHECKING(for a CURLOPT_DEBUGDATA in libcurl)
	AC_TRY_RUN([
	#include <curl/curl.h>
	int
	main()
	{
		CURLOPT_PRIVATE;
		return 0;
	}],[l=yes],[l=no])
	AC_MSG_RESULT($l)
	if [[ $l == "yes" ]]; then
		AC_DEFINE(HAVE_CURLOPT_PRIVATE,[],[libcurl has CURLOPT_DEBUGDATA])
	fi
])
