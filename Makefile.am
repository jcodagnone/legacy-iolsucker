
AUTOMAKE_OPTIONS = foreign dist-bzip2

EXTRA_DIST = configwin.h

SUBDIRS = \
	bin \
	lib \
	src \
	doc \
	win32 \
	iolwizard \
	extra

WINPREFIX = /tmp/test/$(distdir)
win32rel:	
	rm -rf $(WINPREFIX)
	mkdir $(WINPREFIX)
	mkdir $(WINPREFIX)/compile
	cp -R extra/inst/* $(WINPREFIX)
	if [[ -a $(WINPREFIX)/source ]]; then  \
		tar -C $(WINPREFIX)/source -xjf $(distdir).tar.bz2 ; \
	fi
	tar -C $(WINPREFIX)/compile -xjf $(distdir).tar.bz2
