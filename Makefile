BIN?=bin
SRC?=signals/src
EXTRA?=signals/extra

SIGNALS_HAS_X11?=0
SIGNALS_HAS_APPS?=0

include signals/Makefile

#------------------------------------------------------------------------------

ProjectName:=src/
include $(ProjectName)/project.mk

#------------------------------------------------------------------------------

targets: $(TARGETS)
