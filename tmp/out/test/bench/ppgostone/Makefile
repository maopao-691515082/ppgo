.PHONY: all

include Makefile.def

PPGO_CXX := g++
PPGO_CXX_FLAGS := -std=gnu++17 -ggdb3 -O2 \
	-Werror -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable \
	-fPIC -fno-strict-aliasing -fwrapv -pthread -U_FORTIFY_SOURCE

all:
	$(PPGO_CXX) $(PPGO_CXX_FLAGS) -o ./$(PPGO_MK_OUT) *.cpp -lm
