RELAZIONE_SRC_DIR=./relazione
RELAZIONE_SRC_ODTS=$(shell find $(RELAZIONE_SRC_DIR) -name "*.odt")
RELAZIONE_SRC_DOCXS=$(shell find $(RELAZIONE_SRC_DIR) -name "*.docx")
RELAZIONE_PDFS_DIR=$(RELAZIONE_SRC_DIR)/pdf
RELAZIONE_PDFS_FROM_ODTS=$(addsuffix .pdf, $(addprefix $(RELAZIONE_PDFS_DIR)/,$(shell basename -a $(RELAZIONE_SRC_ODTS) 2>/dev/null | awk -F . '{print $$1}')))
RELAZIONE_PDFS_FROM_DOCXS=$(addsuffix .pdf, $(addprefix $(RELAZIONE_PDFS_DIR)/,$(shell basename -a $(RELAZIONE_SRC_DOCXS) 2>/dev/null | awk -F . '{print $$1}')))
RELAZIONE=relazione.pdf
SLIDES_SRC_DIR=./slides
SLIDES_SRC_ODPS=$(shell find $(SLIDES_SRC_DIR) -name "*.odp")
SLIDES_SRC_PPTXS=$(shell find $(SLIDES_SRC_DIR) -name "*.pptx")
SLIDES_PDFS_DIR=$(SLIDES_SRC_DIR)/pdf
SLIDES_PDFS_FROM_ODPS=$(addsuffix .pdf, $(addprefix $(SLIDES_PDFS_DIR)/,$(shell basename -a $(SLIDES_SRC_ODPS) 2>/dev/null | awk -F . '{print $$1}')))
SLIDES_PDFS_FROM_PPTXS=$(addsuffix .pdf, $(addprefix $(SLIDES_PDFS_DIR)/,$(shell basename -a $(SLIDES_SRC_PPTXS) 2>/dev/null | awk -F . '{print $$1}')))
SLIDES=slides.pdf
OUT_DIR=out

all: relazione slides

relazione: $(OUT_DIR)/$(RELAZIONE)
	
$(OUT_DIR)/$(RELAZIONE): $(RELAZIONE_PDFS_FROM_ODTS) $(RELAZIONE_PDFS_FROM_DOCXS)
	mkdir -p $(OUT_DIR)
	gs -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$@ $^	# https://stackoverflow.com/questions/2507766/merge-convert-multiple-pdf-files-into-one-pdf#answer-2507825

$(RELAZIONE_PDFS_FROM_ODTS): $(RELAZIONE_PDFS_DIR)/%.pdf: $(RELAZIONE_SRC_DIR)/%.odt
	mkdir -p $(RELAZIONE_PDFS_DIR)
	libreoffice --convert-to pdf $< --outdir $(RELAZIONE_PDFS_DIR)	# man libreoffice --convert-to

$(RELAZIONE_PDFS_FROM_DOCXS): $(RELAZIONE_PDFS_DIR)/%.pdf: $(RELAZIONE_SRC_DIR)/%.docx
	mkdir -p $(RELAZIONE_PDFS_DIR)
	libreoffice --convert-to pdf $< --outdir $(RELAZIONE_PDFS_DIR)	# man libreoffice --convert-to
	
slides: $(OUT_DIR)/$(SLIDES)
	
$(OUT_DIR)/$(SLIDES): $(SLIDES_PDFS_FROM_ODPS) $(SLIDES_PDFS_FROM_PPTXS)
	mkdir -p $(OUT_DIR)
	gs -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$@ $^

$(SLIDES_PDFS_FROM_ODPS): $(SLIDES_PDFS_DIR)/%.pdf: $(SLIDES_SRC_DIR)/%.odp
	mkdir -p $(SLIDES_PDFS_DIR)
	libreoffice --convert-to pdf $< --outdir $(SLIDES_PDFS_DIR)
	
$(SLIDES_PDFS_FROM_PPTXS): $(SLIDES_PDFS_DIR)/%.pdf: $(SLIDES_SRC_DIR)/%.odp
	mkdir -p $(SLIDES_PDFS_DIR)
	libreoffice --convert-to pdf $< --outdir $(SLIDES_PDFS_DIR)
