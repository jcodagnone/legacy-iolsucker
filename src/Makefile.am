
bin_PROGRAMS = iolsuck link


iolsuck_CFLAGS = @CFLAGS@ @CURL_CFLAGS@

LIBS =  -L. -lefence @LIBS@  @CURL_LIBS@

link_SOURCES = link.c link.h testlink.c link_debug.c

iolsuck_SOURCES = \
	main.cpp 		\
	i18n.h			\
	iol.cpp			\
	iol.h			\
	link.c			\
	link.h			\
	strdup.c		\
	strdup.h		\
	trace.c			\
	trace.h			\
	link_debug.c

link_debug.c:	link.c gendebug.sh
	gendebug.sh < link.c > link_debug.c
	
