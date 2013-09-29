all:
	g++ *.cxx -lglut -lGLU -lGL -orsight -g -O3 `pkg-config --libs --cflags gstreamer-1.0`
