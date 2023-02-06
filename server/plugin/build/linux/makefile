NAME 		= grassroots_plugin
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE :=$(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif
include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib



BUILD		= debug

VPATH	= \
	$(DIR_SRC) \
	$(DIR_SRC)/platform \
	
INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC)
	
SRCS 	= \
	plugin.c \
	unix_plugin.c
	
CPPFLAGS += -DUNIX -DGRASSROOTS_PLUGIN_LIBRARY_EXPORTS

BASE_LDFLAGS = \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \

LDFLAGS += $(BASE_LDFLAGS) 

ifeq ($(BUILD),release)
	CFLAGS 	+= -O3 -s
 	LDFLAGS += -s
else
	CFLAGS 	+= -g
	CPPFLAGS += -D_DEBUG
endif


include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile



