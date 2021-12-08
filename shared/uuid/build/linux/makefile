NAME := grassroots_uuid
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		:= debug

VPATH	:= \
	$(DIR_SRC) \
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_PCRE_INC)
	
	
	
SRCS 	:= \
	uuid_util.c
	

ifeq ($(BUILD),release)
	CFLAGS 	+= -O3 -s
 	LDFLAGS += -s
else
	CFLAGS 	+= -g
	CPPFLAGS += -D_DEBUG
endif
	

CPPFLAGS += -DGRASSROOTS_UUID_LIBRARY_EXPORTS -DUNIX 

LDFLAGS += -ldl \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_PCRE_LIB) -lpcre \
	-L$(DIR_UUID_LIB) -luuid \


HEADERS := $(patsubst %.c, %.h, $(SRCS))
SWIG_HEADERS_TO_IGNORE := \
	alloc_failure.cpp \
	resource.h \

SWIG_HEADERS := $(filter-out $(SWIG_HEADERS_TO_IGNORE),$(HEADERS))

SWIG := /home/billy/Applications/swig/bin/swig
include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

.PHONY:	util all test info swig-interface

util: all

test:
	$(COMP) $(CFLAGS) $(CPPFLAGS) -Wl,--no-as-needed -L$(DIR_OBJS)/ -l$(NAME) -lm  $(INCLUDES)  test.c -o test


info:
	@echo $(SWIG_HEADERS)

swig-interface:
	@echo "%module util" > util.i
	@echo "%{" >> util.i
	@echo "/* Includes the headers in the wrapper code */" >> util.i
	@$(foreach HEADER,$(SWIG_HEADERS), \
   	echo '#include "$(HEADER)"' >> util.i ; )
	@echo "%}" >> util.i
	@echo "\n" >> util.i
	@echo "/* Parse the headers to generate the wrappers */" >> util.i
	@$(foreach HEADER,$(SWIG_HEADERS), \
   	echo '%include "$(HEADER)"' >> util.i ; )
   	
swig-python:
	$(SWIG) -python $(INCLUDES) $(CPPFLAGS) util.i
