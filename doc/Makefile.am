AUTOMAKE_OPTIONS = foreign no-dependencies

EXTRA_DIST =  Doxyfile iolsucker.1.in
man_MANS = iolsucker.1

all: @IOL_HTML_PAGES@ @IOL_PDF_PAGES@

clean:
	$(RM) -f *.html *.pdf *.ps


# MAN2HTML = groff -Thtml -man $< > $@
MAN2HTML = groff -Tascii -man $< |col -b|sed -e 's/</\&lt;/g' -e 's/>/\&gt;/g'|awk 'BEGIN {print"<html><head><title>'$<' man page</title></head><body><pre>"}{print}END{print "</pre></body></html>"}' > $@

SUFFIXES = .1 .html .pdf


.1.html:
	$(MAN2HTML)

%.ps:	%.1
	- groff -Tps -man $< > $@

%.pdf: %.ps
	- ps2pdf $< $@


