all:
	g++ main.cxx method.cxx -lglut -lGLU -lGL -orsight -g -O3 `pkg-config --libs --cflags gstreamer-1.0`

train: train.cxx
	g++ train.cxx -otrain -O3

prereq:
	# gstreamer for sound, pulls in enough C++ for us
	# GLUT for graphics, pulls in all the OpenGL stuff
	# pygtk is already in the default install
	sudo apt-get install libgstreamer1.0-dev freeglut3-dev
