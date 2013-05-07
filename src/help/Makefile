HELPFILE=isla.htb
PDF=isla.pdf

all: $(HELPFILE) $(PDF)

$(HELPFILE): isla.hhp
	zip $(HELPFILE) isla*.html *.hhp *.hhk *.hhc *.gif

$(PDF): isla.tex
	pdflatex isla.tex
	pdflatex isla.tex

isla.hhp: isla.tex tex2rtf.ini
	tex2rtf isla.tex isla.html -html -twice

clean:
	$(RM) isla*.html *.hhp *.hhk *.hhc *.gif
	$(RM) *.con *.ref isla.aux isla.htx isla.log isla.idx isla.toc
	$(RM) $(HELPFILE) $(PDF)
