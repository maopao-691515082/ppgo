.PHONY: all clean

LIB_LOM := lom/build/lom/lib/liblom.a

LIBS := $(LIB_LOM)

all: $(LIBS)

clean:
	make -C lom clean

$(LIB_LOM):
	make -C lom clean all
