dnl AM_PATH_DB([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libdb, and define DB_CFLAGS and DB_LIBS
dnl
AC_DEFUN(AM_PATH_DB,
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(db,[  --with-db=PFX   Prefix where libdb is installed (optional)], db="$withval", db_prefix="")
AC_ARG_WITH(db-libraries,[  --with-db-libraries=DIR   Directory where libdb library is installed (optional)], db_libraries="$withval",
db_libraries="")
AC_ARG_WITH(db-includes,[  --with-db-includes=DIR   Directory where libdb header files are installed (optional)], db_includes="$withval", db_includes="")
AC_ARG_ENABLE(dbtest, [  --without-db               Do not try to compile and
run a test libdb program],, enable_dbtest=yes)

if test "x$db_prefix" != "xno" ; then

  if test "x$db_libraries" != "x" ; then
    DB_LIBS="-L$db_libraries"
  elif test "x$db_prefix" != "x" ; then
    DB_LIBS="-L$db_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    DB_LIBS="-L$prefix/lib"
  fi

  DB_LIBS="$DB_LIBS -ldb"

  if test "x$db_includes" != "x" ; then
    DB_CFLAGS="-I$db_includes"
  elif test "x$db_prefix" != "x" ; then
    DB_CFLAGS="-I$db_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    DB_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for libdb)
  no_db=""


  if test "x$enable_dbtest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $DB_CFLAGS"
    LIBS="$LIBS $DB_LIBS"
dnl
dnl Now check if the installed libdb is sufficiently new.
dnl
      rm -f conf.dbtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <db.h>

int main ()
{
  system("touch conf.dbtest");
  return 0;
}

],, no_db=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_db" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])    
  else
     AC_MSG_RESULT(no)
     if test -f conf.dbtest ; then
       :
     else
       echo "*** Could not run libdb test program, checking why..."
       CFLAGS="$CFLAGS $DB_CFLAGS"
       LIBS="$LIBS $DB_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <db.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually
means"
       echo "*** that the run-time linker is not finding libdb or finding
the wrong"
       echo "*** version of libdb. If it is not finding libdb, you'll need
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
       echo "*** exact error that occured. This usually means libdb was
incorrectly installed"
       echo "*** or that you have moved libdb since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     DB_CFLAGS=""
     DB_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_DEFINE(HAVE_DB, 1, [Define if you have libdb.])
else
  DB_CFLAGS=""
  DB_LIBS=""
fi
  AC_SUBST(DB_CFLAGS)
  AC_SUBST(DB_LIBS)
  rm -f conf.dbtest
])


