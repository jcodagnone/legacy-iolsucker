AM_CFLAGS = @CFLAGS@ -I../lib @CURL_CFLAGS@  @XML_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @LDFLAGS@  -L../lib -L. 

LIBS =  -lmisc @CURL_LIBS@ @LIBS@ -lefence  @XML_LIBS@ @GLIB_LIBS@

bin_PROGRAMS = iolsuck link

link_SOURCES = link.c link.h testlink.c link_debug.c
iolsuck_SOURCES = \
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
	ask_pass.h

link_debug.c:	link.c gendebug.sh
	gendebug.sh < link.c > link_debug.c
	
