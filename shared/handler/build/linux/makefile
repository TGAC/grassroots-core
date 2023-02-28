NAME 		:= grassroots_handler
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL := $(DIR_GRASSROOTS_INSTALL)/lib

BUILD		:= debug

VPATH	:= \
	$(DIR_SRC) 

INCLUDES := \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_IRODS_INC) \
	-I$(DIR_GRASSROOTS_PLUGIN_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC)/parameters \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC)
	
SRCS 	:= \
	handler_utils.c \
	handler.c

CPPFLAGS += -DGRASSROOTS_HANDLER_LIBRARY_EXPORTS

LDFLAGS := -lpthread \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_PLUGIN_LIB) -l$(GRASSROOTS_PLUGIN_LIB_NAME) 


-include $(DIR_BUILD_CONFIG)/irods.mk

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile


