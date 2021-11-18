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
	$(MYDIR)/../signals/src/lib_appcommon/options.cpp\
	$(MYDIR)/mp4_mux_file_handler_dyn.cpp\
	$(MYDIR)/redash.cpp\
	$(MYDIR)/subtitle_source.cpp\
	$(MYDIR)/subtitle_source_synthetic_ttml.cpp\
	$(MYDIR)/subtitle_source_playlist.cpp\
	$(MYDIR)/subtitle_source_playlist_ttml.cpp\
	$(MYDIR)/main.cpp\
	$(MYDIR)/pipeliner.cpp\
	$(MYDIR)/shell.cpp\

$(TARGET): $(EXE_SUBTITLE_LIVE_INSERTER_SRCS:%=$(BIN)/%.o)
