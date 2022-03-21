all: test

ARGS 				:=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	ARGS = $(shell pkg-config pkg-config --cflags fontconfig --libs x11 xext xfixes xrender xcursor xinerama xft)
endif
ifeq ($(UNAME_S),Darwin)
	OSFLAG += -D OSX
endif
test: test.cpp
	g++ test.cpp -lfltk -ldl -lfontconfig -lpthread -lm -lrt ${ARGS} -o test