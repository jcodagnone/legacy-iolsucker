AM_CFLAGS = @CFLAGS@ -I$(srcdir)/../lib  -DNDEBUG $(DB_INCLUDES)
AM_LDFLAGS = @LDFLAGS@  -L$(srcdir)/../lib/libmisc -L$(srcdir)/../lib/libcrash -L.
LIBS =  @LIBS@ $(DB_LIBS)
srcdir = @srcdir@

#########

EXTRA_DIST = win32.h win32.c cache_db.c cache_dummy.c

EXTRA_PROGRAMS = link testuri
bin_PROGRAMS = iolsucker 

#########

link_SOURCES = link.c link.h link_debug.c 
link_CFLAGS = -D LINK_TEST 
link_LDADD = ../lib/libmisc/libmisc.a  ../lib/libcrash/libcrash.a
iolsucker_SOURCES = main.c main.h config.c fconfig.h  iol.c iol.h \
                    link.c link.h getpass.c getpass.h progress.c \
                    progress.h  iolerr.c opt.c forum.c \
                    forum.h demo.c  link_debug.c stringset.c stringset.h \
		    common.h common.c cache.h course.c course.h urihelper.h \
		    urihelper.c

testuri_SOURCES = urihelper.c urihelper.h
testuri_CFLAGS  = -D URIHELPER_DRIVER @GLIB_CFLAGS@  -g
testuri_LDFLAGS = @GLIB_LIBS@ -g
	    
if LIBDB
iolsucker_SOURCES += cache_db.c
else
iolsucker_SOURCES += cache_dummy.c
endif

iolsucker_CFLAGS = $(AM_CFLAGS) @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@ 
iolsucker_LDADD = ../lib/libmisc/libmisc.a ../lib/libcrash/libcrash.a \
                  @XML_LIBS@ @GLIB_LIBS@  @CURL_LIBS@
iolsucker_DEPENDENCIES = ../lib/libmisc/libmisc.a  ../lib/libcrash/libcrash.a

################

link_debug.c:	link.c $(srcdir)/../bin/gen_debug.sh $(srcdir)/../bin/gen_func.sh
	$(srcdir)/../bin/gen_debug.sh < link.c > link_debug.c
	$(srcdir)/../bin/gen_func.sh < link.c >> link_debug.c

iolerr.c:	$(srcdir)/../bin/gen_iol_err.awk $(srcdir)/iol.h
	$(srcdir)/../bin/gen_iol_err.awk < $(srcdir)/iol.h > $(srcdir)/iolerr.c


ioldemo:
	rm -f iolsucker-demo.o iolsucker-iol.o ;
	$(MAKE) CFLAGS="-D IOLDEMO"
