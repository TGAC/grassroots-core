NAME 		= grassroots_task
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/$(PLATFORM))
DIR_SRC := $(realpath $(DIR_BUILD)/../../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../../build-config/unix/)
#export DIR_BUILD_CONFIG = /Users/tyrrells/Projects/grassroots/build-config/unix/
endif

include $(DIR_BUILD_CONFIG)/project.properties
-include $(DIR_BUILD)/user.prefs


export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		= debug

VPATH	= \
	$(DIR_SRC) \
	$(DIR_SRC)/platform \
	
INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_INCLUDE)/platform \
	-I$(DIR_GRASSROOTS_USERS_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_IRODS_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC)/parameters \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_PLUGIN_INC) \
	-I$(DIR_GRASSROOTS_DRMAA_INC) \
	-I$(DIR_GRASSROOTS_MONGODB_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_MONGODB_INC) \
	-I$(DIR_BSON_INC) \
	
	

ifneq ($(BUILD_COMBINED), 1)
SRCS 	= \
	async_task.c \
	async_tasks_manager.c \
	count_async_task.c \
	event_consumer.c \
	system_async_task.c \

SRCS += $(PLATFORM_SRCS)
endif	
	
	
CPPFLAGS += -DGRASSROOTS_TASK_LIBRARY_EXPORTS 

LDFLAGS = -lpthread \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UUID_LIB) -l$(GRASSROOTS_UUID_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_HANDLER_LIB) -l$(GRASSROOTS_HANDLER_LIB_NAME) \
	-L$(DIR_JANSSON_LIB) -ljansson -lcurl \
	-L$(DIR_UUID_LIB) -luuid \
	
include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile	

