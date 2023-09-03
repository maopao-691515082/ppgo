.PHONY: all fast

include Makefile.def
include $(PPGO_DIR)/conf/defs.mk

PPGO_CXX_FLAGS += -I$(PPGO_DIR)/deps/lom/build/lom/include
PPGO_LD_FLAGS := $(PPGO_DIR)/deps/lom/build/lom/lib/liblom.a -lm

all:
	rm -rf *.o
	$(PPGO_CXX) $(PPGO_CXX_FLAGS) -o ./$(PPGO_MK_OUT) *.cpp $(PPGO_LD_FLAGS)

fast:
	python2 make_fast.py \
		'$(PPGO_CXX)' '$(PPGO_CXX_FLAGS)' '$(PPGO_LD_FLAGS)' './$(PPGO_MK_OUT)' \
		'$(PPGO_OUT_OBJ_CACHE_DIR)'
	rm -rf '$(PPGO_OUT_OBJ_CACHE_DIR)'
	mkdir -p '$(PPGO_OUT_OBJ_CACHE_DIR)'
	cp *.o '$(PPGO_OUT_OBJ_CACHE_DIR)/'
	cp hashes '$(PPGO_OUT_OBJ_CACHE_DIR)/'
