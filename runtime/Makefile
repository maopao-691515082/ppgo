.PHONY: all

include defs.mk
include Makefile.def

PPGO_CXX_FLAGS += -I$(PPGO_DEPS_DIR)/lom/build/lom/include
PPGO_LD_FLAGS := $(PPGO_DEPS_DIR)/lom/build/lom/lib/liblom.a -lm

all:
	$(PPGO_CXX) $(PPGO_CXX_FLAGS) -o ./$(PPGO_MK_OUT) *.cpp $(PPGO_LD_FLAGS)
