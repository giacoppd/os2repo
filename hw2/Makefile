LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF	= dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

all: pdf phil

pdf:
	latex os2hw2.tex
	dvips -R -Poutline -t letter os2hw2.dvi -o os2hw2.ps
	ps2pdf os2hw2.ps

phil:
	gcc -o phil phil.c -Wall -pthread

clean:
	rm -f *.dvi *.out *.aux *.log *.bbl *.blg *.ps *.pdf phil
