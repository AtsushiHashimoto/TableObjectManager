OBJS=$(patsubst %.cpp,%.o,$(shell find ./ -name "*.cpp" -print))

LIBNAME = libskl_core

ADDITIONALMODULE = 

L_BITS := $(shell getconf LONG_BIT)

CC = g++
CFLAGS = -Wall -std=c++0x
CFLAGS_D = -Wall -std=c++0x


INC += -I../include -I../../../include
INC += ${ADDITIONALMODULE:%=-I../../%/include/}

.SUFFIXES:.cpp

.PHONY:clean debug

default:${LIBNAME}.a

debug:${LIBNAME}_d.a

${LIBNAME}.a:${OBJS:%=${LIBNAME}.a(%)}

${LIBNAME}(%.o):%.o

${LIBNAME}_d.a:${OBJS:%.o=${LIBNAME}_d.a(%_d.o)}

${LIBNAME}_d(%_d.o):%_d.o

# hoge.cpp -> hoge.o
.cpp.o:
	$(CC) -O2 -c $(CFLAGS) -o $@ $< $(INC) -D__linux__

# hoge.cpp -> hoge_d.o
%_d.o:%.cpp
	$(CC) -g -c $(CFLAGS) -o $@ $< $(INC) -D_DEBUG -D__linux__

clean:
	rm -f *.o
	rm -f $(LIBNAME).a
	rm -f $(LIBNAME)_d.a
