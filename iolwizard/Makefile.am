
AM_CFLAGS = @CFLAGS@ -I$(srcdir)/../lib  @XML_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @LDFLAGS@  -L../lib -L.  
LIBS =  @LIBS@  @XML_LIBS@ @GTK_LIBS@ @GLIB_LIBS@
srcdir = @srcdir@

EXTRA_DIST = dirent.c dirent.h
EXTRA_PROGRAMS = iolwizard
bin_PROGRAMS = @IOLWIZARD@

noinst_HEADERS = ui.h dirbrowser.h error_dlg.h
iolwizard_SOURCES = main.c ui.c dirbrowser.c  error_dlg.c
iolwizard_CFLAGS = $(AM_CFLAGS)
iolwizard_LDADD = ../src/iolsucker-opt.o ../src/iolsucker-config.o ../lib/libmisc.a 
iolwizard_DEPENDENCIES = ../lib/libmisc.a
