
AUTOMAKE_OPTIONS = foreign dist-bzip2

EXTRA_DIST = TODO NOTES autogen.sh

SUBDIRS = bin lib src doc win32 iolwizard iolunch extra debian

WINPREFIX = /tmp/iol/$(distdir)
windist:	dist	doc/iolsucker.html
	rm -rf $(WINPREFIX)
	mkdir -p $(WINPREFIX)
	mkdir $(WINPREFIX)/compile
	cp -R extra/inst/* $(WINPREFIX)
	cp doc/iolsucker.html $(WINPREFIX)/data
	if [[ -d $(WINPREFIX)/source ]]; then  \
		tar -C $(WINPREFIX)/source -xjf $(distdir).tar.bz2 ; \
	fi
	tar -C $(WINPREFIX)/compile -xjf $(distdir).tar.bz2
	if [[ -d $(WINPREFIX)/compile ]] ; then \
		cp -R unix $(WINPREFIX)/compile ; \
		cp unix/dll/*.dll $(WINPREFIX)/data; \
	fi 
	chmod o+w -R $(WINPREFIX)
