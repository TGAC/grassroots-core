NAME 		= grassroots_service
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/$(PLATFORM))
DIR_SRC := $(realpath $(DIR_BUILD)/../../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../../build-config/unix/)
#export DIR_BUILD_CONFIG = /Users/tyrrells/Projects/grassroots/build-config/unix/
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib



BUILD		:= debug

VPATH	:= \
	$(DIR_SRC) \
	$(DIR_SRC)/platform \
	$(DIR_SRC)/parameters \

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_INCLUDE)/platform \
	-I$(DIR_INCLUDE)/parameters \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_USERS_INC) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_PLUGIN_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_BSON_INC)
	
ifneq ($(BUILD_COMBINED), 1)
	
SRCS 	= \
	encode_dcc_metadata.c \
	linked_service.c \
	mapped_parameter.c \
	paired_service.c \
	provider.c \
	remote_service_job.c \
	schema_term.c \
	service.c \
	service_job.c \
	service_job_set_iterator.c \
	service_metadata.c \
	web_service_util.c

SRCS 	+= \
	boolean_parameter.c \
	char_parameter.c \
	double_parameter.c \
	json_parameter.c \
	parameter.c \
	parameter_group.c \
	parameter_set.c \
	remote_parameter_details.c \
	resource_parameter.c \
	signed_int_parameter.c \
	string_parameter.c \
	string_array_parameter.c \
	time_parameter.c \
	time_array_parameter.c \
	unsigned_int_parameter.c 


endif	
		

CPPFLAGS +=  -DGRASSROOTS_SERVICE_LIBRARY_EXPORTS

BASE_LDFLAGS = -lpthread \
	-L$(DIR_GRASSROOTS_PLUGIN_LIB) -l$(GRASSROOTS_PLUGIN_LIB_NAME) \
	-L$(DIR_GRASSROOTS_HANDLER_LIB) -l$(GRASSROOTS_HANDLER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UUID_LIB) -l$(GRASSROOTS_UUID_LIB_NAME) \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_UUID_LIB) -luuid \
	-lcurl

LDFLAGS += $(BASE_LDFLAGS) 

ifeq ($(BUILD),release)
	CFLAGS 	+= -O3 -s
 	LDFLAGS += -s
else
	CFLAGS 	+= -g
	CPPFLAGS += -D_DEBUG
endif
	

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile




