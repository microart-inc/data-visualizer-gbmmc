all: test

ARGS = $(shell pkg-config pkg-config --cflags fontconfig --libs x11 xext xfixes xrender xcursor xinerama xft)
test: test.cpp
	g++ test.cpp -lfltk -ldl -lfontconfig -lpthread -lm -lrt ${ARGS} -o test