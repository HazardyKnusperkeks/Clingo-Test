TEMPLATE	 = app
CONFIG		*= console c++1z
CONFIG		-= app_bundle qt

SOURCES		 = main.cpp

INCLUDEPATH	*= $(HOME)/clingo/libclasp\
			   $(HOME)/clingo/libclingo\
			   $(HOME)/clingo/libgringo\
			   $(HOME)/clingo/liblp\
			   $(HOME)/clingo/libprogram_opts

LIBS		*= -L$(HOME)/clingo/build/debug/\
			   -Wl,-rpath=$(HOME)/clingo/build/debug/\
			   -lclingo

DEFINES		*= WITH_THREADS=1

DESTDIR		 = ../bin
