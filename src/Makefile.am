AM_CFLAGS = @CFLAGS@ -I../lib  -DNDEBUG
AM_LDFLAGS = @LDFLAGS@  -L../lib -L. 
LIBS =  @LIBS@ -ldb 

#########

EXTRA_DIST = win32.h win32.c 

EXTRA_PROGRAMS = link
bin_PROGRAMS = iolsucker 

#########

link_SOURCES = link.c link.h link_debug.c
link_CFLAGS = -D LINK_TEST
link_LDADD = ../lib/libmisc.a 
iolsucker_SOURCES = main.c main.h config.c fconfig.h  iol.c iol.h \
                    link.c link.h getpass.c getpass.h progress.c \
                    progress.h  iolerr.c opt.c cache.c cache.h forum.c \
                    forum.h demo.c  link_debug.c
iolsucker_CFLAGS = $(AM_CFLAGS) @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@ 
iolsucker_LDADD = ../lib/libmisc.a @XML_LIBS@ @GLIB_LIBS@  @CURL_LIBS@
iolsucker_DEPENDENCIES = ../lib/libmisc.a

################

link_debug.c:	link.c ../bin/gen_debug.sh ../bin/gen_func.sh
	../bin/gen_debug.sh < link.c > link_debug.c
	../bin/gen_func.sh < link.c >> link_debug.c

iolerr.c:	../bin/gen_iol_err.awk iol.h
	../bin/gen_iol_err.awk < iol.h > iolerr.c


ioldemo:
	rm -f iolsucker-demo.o iolsucker-iol.o ;
	$(MAKE) CFLAGS="-D IOLDEMO"
