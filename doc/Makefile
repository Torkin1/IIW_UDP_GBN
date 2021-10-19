RELAZIONE_SRC_DIR=./relazione
RELAZIONE_SRC=$(shell find $(RELAZIONE_SRC_DIR) -name "*.odt" -o -name "*.docx" -o -name "*.doc")
RELAZIONE_PDFS_DIR=$(RELAZIONE_SRC_DIR)/pdf
RELAZIONE_PDFS=$(shell find $(RELAZIONE_PDFS_DIR) -name "*.pdf")
RELAZIONE=relazione.pdf
SLIDES_SRC_DIR=./slides
SLIDES_SRC=$(shell find $(SLIDES_SRC_DIR) -name "*.odp" -o -name "*.pptx" -o -name "*.ppt")
SLIDES_PDFS_DIR=$(SLIDES_SRC_DIR)/pdf
SLIDES_PDFS=$(shell find $(SLIDES_PDFS_DIR) -name "*.pdf")
SLIDES=slides.pdf
OUT_DIR=out

all: relazione slides

relazionePDFs: $(RELAZIONE_SRC)
	mkdir -p $(RELAZIONE_PDFS_DIR)
	rm -f $(RELAZIONE_PDFS_DIR)/*
	libreoffice --convert-to pdf $(RELAZIONE_SRC) --outdir $(RELAZIONE_PDFS_DIR)	# man libreoffice --convert-to
	
relazione: relazionePDFs
	mkdir -p $(OUT_DIR)
	gs -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$(OUT_DIR)/$(RELAZIONE) $(RELAZIONE_PDFS)	# https://stackoverflow.com/questions/2507766/merge-convert-multiple-pdf-files-into-one-pdf#answer-2507825
	rm -r $(RELAZIONE_PDFS_DIR)

slidesPDFs: $(SLIDES_SRC)
	mkdir -p $(SLIDES_PDFS_DIR)
	rm -f $(RELAZIONE_PDFS_DIR)/*
	libreoffice --convert-to pdf $(SLIDES_SRC) --outdir $(SLIDES_PDFS_DIR)
	
slides: slidesPDFs
	mkdir -p $(OUT_DIR)
	gs -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$(OUT_DIR)/$(SLIDES) $(SLIDES_PDFS)
	rm -r $(SLIDES_PDFS_DIR)
