.PHONY: all clean

include defs.mk

all:
	@rm -rf build/tmpobjs build/lom build/test
	@mkdir -p build/tmpobjs build/lom/lib build/test
	@cp -r include defs.mk build/lom/
	@cd build && rm -rf LOM && ln -s .. LOM
	@python make.py \
		'$(LOM_CXX)' \
		'$(LOM_CXX_FLAGS)' \
		'$(LOM_AR)' \
		'$(LOM_AR_FLAGS)' \
		'$(LOM_LD)' \
		'$(LOM_LD_FLAGS)' \
		'$(LIM_LD_STD_LIB_FLAGS)'
	@rm -rf build/last_tmpobjs
	@cp -r build/tmpobjs build/last_tmpobjs

clean:
	@rm -rf build
