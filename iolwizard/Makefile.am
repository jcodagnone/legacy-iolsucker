
AM_CFLAGS = @CFLAGS@ -I../lib  @XML_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @LDFLAGS@  -L../lib -L.  

LIBS =  @LIBS@  @XML_LIBS@ @GTK_LIBS@ @GLIB_LIBS@

EXTRA_PROGRAMS = iolwizard
bin_PROGRAMS = @IOLWIZARD@

noinst_HEADERS = ui.h
iolwizard_SOURCES = main.c ui.c
iolwizard_LDADD = ../lib/libmisc.a
iolwizard_DEPENDENCIES = ../lib/libmisc.a
