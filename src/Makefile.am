AM_CFLAGS = @CFLAGS@ -I../lib @CURL_CFLAGS@  `xml2-config  --cflags`

AM_LDFLAGS = @LDFLAGS@

LIBS =  -lmisc @CURL_LIBS@ @LIBS@ -lefence  `xml2-config --libs`
LDFLAGS = -L../lib -L. 

bin_PROGRAMS = iolsuck link

link_SOURCES = link.c link.h testlink.c link_debug.c
iolsuck_SOURCES = \
	main.cpp 		\
	main.h			\
	config.c		\
	config.h		\
	i18n.h			\
	iol.cpp			\
	iol.h			\
	link.c			\
	link.h			\
	link_debug.c		\
	ask_pass.c		\
	ask_pass.h

link_debug.c:	link.c gendebug.sh
	gendebug.sh < link.c > link_debug.c
	
