LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF	= dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

all: pdf

pdf:
	latex os2hw1.tex
	dvips -R -Poutline -t letter os2hw1.dvi -o os2hw1.ps
	ps2pdf os2hw1.ps
 
clean:
	rm -f *.dvi *.out *.aux *.log *.bbl *.blg *.ps *.pdf
