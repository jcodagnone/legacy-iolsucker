
AUTOMAKE_OPTIONS = foreign dist-bzip2

EXTRA_DIST = configwin.h

SUBDIRS = \
	bin \
	lib \
	src \
	doc \
	win32 \
	iolwizard \
	iolunch \
	extra

WINPREFIX = /tmp/iol/$(distdir)
windist:	dist	doc/iolsucker.html
	rm -rf $(WINPREFIX)
	mkdir -p $(WINPREFIX)
	mkdir $(WINPREFIX)/compile
	cp -R extra/inst/* $(WINPREFIX)
	cp doc/iolsucker.html $(WINPREFIX)/data
	if [[ -d $(WINPREFIX)/source ]]; then  \
		tar -C $(WINPREFIX)/source -xjf $(distdir).tar.bz2 ; \
		if [[ -d unix ]] ; then \
			cp -R unix $(WINPREFIX)/source ; \
			cp unix/dll/*.dll $(WINPREFIX)/data; \
		fi \
	fi
	tar -C $(WINPREFIX)/compile -xjf $(distdir).tar.bz2
	chmod o+w -R $(WINPREFIX)
