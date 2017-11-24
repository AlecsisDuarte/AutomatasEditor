OBJS = Editor.o TextWindow.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
# LDLIBS = $(shell pkg-config gtkmm-3.0 --cflags --libs)
PACKAGE = `pkg-config gtkmm-3.0 --cflags --libs`

TextIDE : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o TextIDE $(PACKAGE)

TextWindow.o : TextWindow.h TextWindow.cc
	$(CC) $(PACKAGE) $(CFLAGS) TextWindow.cc

Editor.o : TextWindow.h Editor.cc
	$(CC) $(PACKAGE) $(CFLAGS) Editor.cc

clean : 
	\rm *.o TextIDE