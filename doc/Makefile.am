AUTOMAKE_OPTIONS = foreign no-dependencies

EXTRA_DIST =  Doxyfile iolsucker.1.in
man_MANS = iolsucker.1

all: @IOL_HTML_PAGES@ @IOL_PDF_PAGES@

clean:
	$(RM) -f *.html *.pdf *.ps


MAN2HTML = groff -Thtml -man $< > $@

SUFFIXES = .1 .html .pdf


.1.html:
	$(MAN2HTML)

%.ps:	%.1
	- groff -Tps -man $< > $@

%.pdf: %.ps
	- ps2pdf $< $@


