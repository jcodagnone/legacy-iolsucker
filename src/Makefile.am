AM_CFLAGS = @CFLAGS@ -I../lib @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @LDFLAGS@  -L../lib -L. 

LIBS =  @CURL_LIBS@ @LIBS@  @XML_LIBS@ @GLIB_LIBS@ -lefence

bin_PROGRAMS = iolsucker link

link_SOURCES = link.c link.h testlink.c link_debug.c
iolsucker_SOURCES = \
	main.c 			\
	main.h			\
	config.c		\
	config.h		\
	i18n.h			\
	iol.c			\
	iol.h			\
	link.c			\
	link.h			\
	link_debug.c		\
	ask_pass.c		\
	ask_pass.h		\
	progress.c

iolsucker_LDADD = ../lib/libmisc.a

link_debug.c:	link.c gendebug.sh
	gendebug.sh < link.c > link_debug.c
	
