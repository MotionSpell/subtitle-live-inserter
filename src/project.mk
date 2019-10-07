MYDIR=$(call get-my-dir)
TARGET:=$(BIN)/subtitle-live-inserter.exe
TARGETS+=$(TARGET)

EXE_SUBTITLE_LIVE_INSERTER_MODULES_SRCS:=\
	$(MYDIR)/main.cpp\
	$(MYDIR)/source.cpp\

EXE_SUBTITLE_LIVE_INSERTER_SRCS:=\
	$(LIB_MEDIA_SRCS)\
	$(LIB_MODULES_SRCS)\
	$(LIB_PIPELINE_SRCS)\
	$(LIB_UTILS_SRCS)\
	$(MYDIR)/../signals/src/lib_appcommon/safemain.cpp\
	$(MYDIR)/subtitle_source.cpp\
	$(MYDIR)/main.cpp\
	$(MYDIR)/pipeliner.cpp\

$(TARGET): $(EXE_SUBTITLE_LIVE_INSERTER_SRCS:%=$(BIN)/%.o)
