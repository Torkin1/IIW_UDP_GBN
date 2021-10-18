RELAZIONE_SRC_DIR=./relazione
RELAZIONE_SRC=$(shell find $(RELAZIONE_SRC_DIR) -name "*.odt")
RELAZIONE_PDFS_DIR=$(RELAZIONE_SRC_DIR)/pdf
RELAZIONE_PDFS=$(shell find $(RELAZIONE_OUT_DIR) -name "*.pdf")
RELAZIONE=relazione.pdf
OUT_DIR=out

all: relazione 

relazionePDFs: $(RELAZIONE_SRC)
	mkdir -p $(RELAZIONE_PDFS_DIR)
	rm -f $(RELAZIONE_PDFS_DIR)/*
	libreoffice --convert-to pdf $(RELAZIONE_SRC) --outdir $(RELAZIONE_PDFS_DIR)	# man libreoffice --convert-to
	
relazione: relazionePDFs
	mkdir -p $(OUT_DIR)
	gs -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$(OUT_DIR)/$(RELAZIONE) $(RELAZIONE_PDFS)	# https://stackoverflow.com/questions/2507766/merge-convert-multiple-pdf-files-into-one-pdf#answer-2507825
	rm -r $(RELAZIONE_PDFS_DIR)
