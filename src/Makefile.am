EXTRA_DIST = gencurlerr.awk gendebug.sh
AM_CFLAGS = @CFLAGS@ -I../lib 
AM_LDFLAGS = @LDFLAGS@  -L../lib -L. 
LIBS =  @LIBS@  -lefence

#########

EXTRA_PROGRAMS = link
bin_PROGRAMS = iolsucker 

#########

link_SOURCES = link.c link.h testlink.c link_debug.c
iolsucker_SOURCES = main.c main.h config.c config.h  iol.c iol.h \
                    link.c link.h link_debug.c getpass.c getpass.h progress.c \
                    progress.h curlerr.c
iolsucker_CFLAGS = @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@ 
iolsucker_LDADD = ../lib/libmisc.a @XML_LIBS@ @GLIB_LIBS@  @CURL_LIBS@
iolsucker_DEPENDENCIES = ../lib/libmisc.a

################

link_debug.c:	link.c gendebug.sh
	gendebug.sh < link.c > link_debug.c

curlerr.c:	gencurlerr.awk
	gencurlerr.awk < "`curl-config --prefix`/include/curl/curl.h" >curlerr.c
