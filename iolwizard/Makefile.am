
AM_CFLAGS = @CFLAGS@ -I../lib  @XML_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @LDFLAGS@  -L../lib -L.  

LIBS =  @LIBS@  @XML_LIBS@ @GTK_LIBS@ @GLIB_LIBS@

EXTRA_DIST = dirent.c dirent.h
EXTRA_PROGRAMS = iolwizard
bin_PROGRAMS = @IOLWIZARD@

noinst_HEADERS = ui.h dirbrowser.h
iolwizard_SOURCES = main.c ui.c dirbrowser.c 
iolwizard_CFLAGS = $(AM_CFLAGS)
iolwizard_LDADD = ../src/iolsucker-opt.o ../src/iolsucker-config.o ../lib/libmisc.a 
iolwizard_DEPENDENCIES = ../lib/libmisc.a
