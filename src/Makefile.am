AM_CFLAGS = @CFLAGS@ -I$(srcdir)/../lib  -DNDEBUG $(DB_INCLUDES)
AM_LDFLAGS = @LDFLAGS@  -L$(srcdir)/../lib -L.
LIBS =  @LIBS@ $(DB_LIBS)
srcdir = @srcdir@

#########

SUBDIRS = demo
EXTRA_DIST = win32.h win32.c

EXTRA_PROGRAMS = link
bin_PROGRAMS = iolsucker 

#########

link_SOURCES = link.c link.h link_debug.c 
link_CFLAGS = -D LINK_TEST
link_LDADD = ../lib/libmisc.a 
iolsucker_SOURCES = main.c main.h config.c fconfig.h  iol.c iol.h \
                    link.c link.h getpass.c getpass.h progress.c \
                    progress.h  iolerr.c opt.c forum.c \
                    forum.h demo.c  link_debug.c stringset.c stringset.h \
		    common.h common.c cache.c cache.h

iolsucker_CFLAGS = $(AM_CFLAGS) @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@ 
iolsucker_LDADD = ../lib/libmisc.a @XML_LIBS@ @GLIB_LIBS@  @CURL_LIBS@
iolsucker_DEPENDENCIES = ../lib/libmisc.a

################

link_debug.c:	link.c $(srcdir)/../bin/gen_debug.sh $(srcdir)/../bin/gen_func.sh
	$(srcdir)/../bin/gen_debug.sh < link.c > link_debug.c
	$(srcdir)/../bin/gen_func.sh < link.c >> link_debug.c

iolerr.c:	$(srcdir)/../bin/gen_iol_err.awk $(srcdir)/iol.h
	$(srcdir)/../bin/gen_iol_err.awk < $(srcdir)/iol.h > $(srcdir)/iolerr.c


ioldemo:
	rm -f iolsucker-demo.o iolsucker-iol.o ;
	$(MAKE) CFLAGS="-D IOLDEMO"
