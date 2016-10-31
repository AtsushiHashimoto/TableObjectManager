CUFILES=$(shell find ./ -name "*.cu" -print)
LIBNAME ?= libskl_opencvgpu
STATIC_LIB = ${LIBNAME}.a

SRCDIR = ./
ROOTDIR = ..
ROOTBINDIR = ../
BINDIR = ../
ROOTOBJDIR = ./

INCLUDES += `pkg-config --cflags opencv` -I../../include

# options for OpenCV
piriod:= .
empty:=
space:= $(empty) $(empty)
OPENCVVER	= $(subst $(piriod),$(space),$(shell pkg-config --modversion opencv))
CFLAGS += -DCV_MAJOR_VERSION=$(word 1,$(OPENCVVER))
CFLAGS += -DCV_MINOR_VERSION=$(word 2,$(OPENCVVER))
CFLAGS += -DHAVE_TBB
CFLAGS_D += -DCV_MAJOR_VERSION=$(word 1,$(OPENCVVER))
CFLAGS_D += -DCV_MINOR_VERSION=$(word 2,$(OPENCVVER))
CFLAGS_D += -DHAVE_TBB

NVCCFLAGS += -DCV_MAJOR_VERSION=$(word 1,$(OPENCVVER))
NVCCFLAGS += -DCV_MINOR_VERSION=$(word 2,$(OPENCVVER))

include ./common.mk

.PHONY:debug
debug:
	make dbg=1 LIBNAME=libskl_opencvgpu_d
