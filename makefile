#Compiler
CC = gcc

#Library name
APPNAME = extcurses
LIBNAME = $(APPNAME).a

#libraries needed
LIBS = -lncurses

all: $(LIBNAME)

$(LIBNAME): $(LIBNAME)(menu.o)
	#Built in rule will create the archive

menu.o: menu.c menu.h
	gcc -o menu.o -c menu.c	

clean:
	-rm menu.o $(LIBNAME)

#Targets for distribution only
dist: $(APPNAME)-1.0.tar.gz
	
$(APPNAME)-1.0.tar.gz: $(LIBNAME)	
	-rm -rf $(APPNAME)-1.0 #Directory name
	-mkdir $(APPNAME)-1.0
	-cp *.h *.c makefile $(APPNAME)-1.0
	tar zcvf $@ $(APPNAME)-1.0
	
