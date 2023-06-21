.PHONY: all

include Makefile.def

PPGO_CXX := g++
PPGO_CXX_FLAGS := -std=gnu++17 -ggdb3 -O2 \
	-Werror -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable \
	-fPIC -fno-strict-aliasing -fwrapv -pthread -U_FORTIFY_SOURCE -I$(PPGO_DEPS_DIR)/lom/build/lom/include
PPGO_LD_FLAGS := $(PPGO_DEPS_DIR)/lom/build/lom/lib/liblom.a -lm

all:
	$(PPGO_CXX) $(PPGO_CXX_FLAGS) -o ./$(PPGO_MK_OUT) *.cpp $(PPGO_LD_FLAGS)
