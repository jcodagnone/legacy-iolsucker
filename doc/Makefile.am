AUTOMAKE_OPTIONS = foreign no-dependencies

EXTRA_DIST = \
	Doxygen

man_MANS = \
	iolsucker.1

HTMLPAGES = \
	iolsucker.html

PDFPAGES = \
	iolsucker.pdf
	
all: $(HTMLPAGES) $(PDFPAGES)

clean:
	$(RM) -f *.html *.pdf *.ps


MAN2HTML = groff -Thtml -man $< > $@

SUFFIXES = .1 .html .pdf


.1.html:
	$(MAN2HTML)

%.ps:	%.1
	groff -Tps -man $< > $@

%.pdf: %.ps
	ps2pdf $< $@


