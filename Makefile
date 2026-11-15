##############################################################################
## Makefile for the elevator simulator in 8th
## Tammy Cravit <tammy@tammymakesthings.com>
##############################################################################

MAKEFILE_DIR 	:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
EIGHTH       	:= $(shell command -v 8th 2>/dev/null)
EIGHTH_HOME  	:= $(HOME)/8th
SOURCE_FILES 	:= $(MAKEFILE_DIR)/elevator.8th
MAIN_SCRIPT 	:= $(lastword $(SOURCE_FILES))
MANIFEST_FILES 	:= $(MAKEFILE_DIR)/8th.proj
BUILD_DIR	:= $(MAKEFILE_DIR)/out
BUILD_SCRIPT	:= $(EIGHTH_HOME)/bin/build
BUILD_OPTIONS	:= -g
RUN_OPTIONS	:= -q

bold 		:= $(shell tput setaf 4)
sgr0 		:= $(shell tput sgr0)
smul		:= $(shell tput smul)
rmul		:= $(shell tput rmul)
green		:= $(shell tput setaf 2)

run:	$(SOURCE_FILES)		## Run the script in the 8th interpreter
	$(EIGHTH) $(RUN_OPTIONS) $(MAIN_SCRIPT)

build:	$(SOURCE_FILES)		## Build the executables in the 'out/' directory
ifndef EIGHTH
    $(error "8th is not available or not found - please install it")
endif
	$(EIGHTH) $(BUILD_SCRIPT) $(BUILD_OPTIONS) $(MAKEFILE_DIR)

clean:				## Clean up the build directory
	rm -fr $(BUILD_DIR)

config:				## Display the Makefile configuration
	@echo "$(green)===========================================$(sgr0)"
	@echo "$(green)M A K E F I L E   C O N F I G U R A T I O N$(sgr0)"
	@echo "$(green)===========================================$(sgr0)"
	@echo ""
	@echo "$==> $(smul)Source/Tool Locations$(rmul)"
	@echo ""
	@echo "   $(bold)MAKEFILE_DIR$(sgr0)         = $(MAKEFILE_DIR)"
	@echo "   $(bold)EIGHTH$(sgr0)               = $(EIGHTH)"
	@echo "   $(bold)EIGHTH_HOME$(sgr0)          = $(EIGHTH_HOME)"
	@echo "   $(bold)SOURCE_FILES$(sgr0)         = $(SOURCE_FILES)"
	@echo "   $(bold)MANIFEST_FILES$(sgr0)       = $(MANIFEST_FILES)"
	@echo ""
	@echo "$==> $(smul)Run Options$(rmul)"
	@echo ""
	@echo "   $(bold)RUN_OPTIONS$(sgr0)          = $(RUN_OPTIONS)"
	@echo "   $(bold)MAIN_SCRIPT$(sgr0)          = $(MAIN_SCRIPT)"
	@echo ""
	@echo "$==> $(smul)Build Options$(rmul)"
	@echo ""
	@echo "   $(bold)BUILD_DIR$(sgr0)            = $(BUILD_DIR)"
	@echo "   $(bold)BUILD_SCRIPT$(sgr0)         = $(BUILD_SCRIPT)"
	@echo "   $(bold)BUILD_OPTIONS$(sgr0)        = $(BUILD_OPTIONS)"
	@echo ""

help: Makefile			## Show this help
	@echo "$(green)=================================$(sgr0)"
	@echo "$(green)   M A K E F I L E   H E L P $(sgr0)"
	@echo "$(green)=================================$(sgr0)"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""

.PHONY: build clean help run config

