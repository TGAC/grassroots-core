NAME 		= grassroots_mongodb
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)
COMP		= gcc
LINK		= ld
MAKEDEPEND 	= gcc -MM -MF


ifeq ($(DIR_BUILD_UTILS),)
export DIR_BUILD_UTILS = $(realpath $(DIR_BUILD)/../../../../../build_utils/linux)
endif
include $(DIR_BUILD_UTILS)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		= debug

VPATH	= \
	$(DIR_SRC) 
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_MONGODB_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_BSON_INC) \
	-I$(DIR_JANSSON_INC)
	
SRCS 	= \
	mongodb_tool.c
	

BASE_LDFLAGS = -ldl \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_MONGODB_LIB) -lmongoc-1.0 \
	-L$(DIR_BSON_LIB) -lbson-1.0 
	

CPPFLAGS += -DGRASSROOTS_MONGODB_LIBRARY_EXPORTS

LDFLAGS += $(BASE_LDFLAGS)

include $(DIR_BUILD_UTILS)/generic_makefiles/shared_library.makefile


