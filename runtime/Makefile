.PHONY: all

include Makefile.def
include $(PPGO_DIR)/conf/defs.mk

PPGO_CXX_FLAGS += -I$(PPGO_DIR)/deps/lom/build/lom/include
PPGO_LD_FLAGS := $(PPGO_DIR)/deps/lom/build/lom/lib/liblom.a -lm

all:
	$(PPGO_CXX) $(PPGO_CXX_FLAGS) -o ./$(PPGO_MK_OUT) *.cpp $(PPGO_LD_FLAGS)
