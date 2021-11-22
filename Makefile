BIN?=bin
SRC?=signals/src
EXTRA?=signals/extra

SIGNALS_HAS_X11?=0
SIGNALS_HAS_APPS?=0

#------------------------------------------------------------------------------

EXE_OTHER_SRCS+=$(shell find src/ -path "*/unittests/*.cpp" | sort)
EXE_OTHER_SRCS+=src/re_dash.cpp src/re_hls.cpp

include signals/Makefile

#------------------------------------------------------------------------------

ProjectName:=src/
include $(ProjectName)/project.mk

#------------------------------------------------------------------------------

targets: $(TARGETS)
