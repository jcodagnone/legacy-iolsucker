
AM_CFLAGS = @CFLAGS@ -I../lib 
AM_LDFLAGS = @LDFLAGS@

EXTRA_DIST = dll.c

noinst_HEADERS = newopt.h strdup.h trace.h basename.h unix.h mkrdir.h queue.h \
                 dirname.h i18n.h dll.h
                 
noinst_LIBRARIES = libmisc.a

libmisc_a_SOURCES = newopt.c strdup.c trace.c basename.c mkrdir.c queue.c \
                    dirname.c

