TEMPLATE	 = app
CONFIG		*= console c++1z
CONFIG		-= app_bundle
QT			 = core gui widgets

SOURCES		 = main.cpp

INCLUDEPATH	*= $(HOME)/clingo/libclasp\
			   $(HOME)/clingo/libclingo\
			   $(HOME)/clingo/libgringo\
			   $(HOME)/clingo/liblp\
			   $(HOME)/clingo/libprogram_opts

LIBS		*= -L$(HOME)/clingo/build/debug/\
			   -lclingo\
			   -lclasp\
			   -lgringo\
			   -llp\
			   -lprogram_opts\
			   -lreify

LIBS		*= -lpthread\
			   -lpython2.7\
			   -llua5.1-c++

DEFINES		*= WITH_THREADS=1

TARGET		 = Clingo-Test-C
DESTDIR		 = ../bin

OTHER_FILES	 = ../program/graph_wg.lp\
			   ../program/mailbot.lp

