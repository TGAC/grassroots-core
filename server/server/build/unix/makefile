NAME 		= grassroots_server
EXE_NAME = server
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/$(PLATFORM))
DIR_SRC := $(realpath $(DIR_BUILD)/../../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../../build-config/unix/)
#export DIR_BUILD_CONFIG = /Users/tyrrells/Projects/grassroots/build-config/unix/
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		= debug

VPATH	= \
	$(DIR_SRC) \
	$(DIR_SRC)/platform 
	
INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_INCLUDE)/platform \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC)/parameters \
	-I$(DIR_GRASSROOTS_USERS_INC) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_IRODS_INC) \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_PLUGIN_INC) \
	-I$(DIR_GRASSROOTS_DRMAA_INC) \
	-I$(DIR_GRASSROOTS_MONGODB_INC) \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_MONGODB_INC) \
	-I$(DIR_BSON_INC) \
	
SRCS 	= \
	audit.c \
	jobs_manager.c \
	permission.c \
	system_util.c \
	servers_manager.c \
	service_matcher.c \
	grassroots_server.c \
	providers_state_table.c \
	service_util.c \
	
ifeq ($(BUILD_COMBINED), 1)

VPATH += \
	$(DIR_GRASSROOTS_SERVICES)/src \
	$(DIR_GRASSROOTS_SERVICES)/src/parameters \
	$(DIR_GRASSROOTS_TASK)/src \
	$(DIR_GRASSROOTS_TASK)/src/platform \

INCLUDES += \
	-I$(DIR_GRASSROOTS_TASK_INC)/platform \


# TASK
SRCS 	+= \
	async_task.c \
	async_tasks_manager.c \
	count_async_task.c \
	event_consumer.c \
	system_async_task.c 

# SERVICES
SRCS 	+= \
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

# PARAMETERS
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


CPPFLAGS += -DGRASSROOTS_SERVICE_LIBRARY_EXPORTS -DGRASSROOTS_TASK_LIBRARY_EXPORTS


endif




SRCS += $(PLATFORM_SRCS)

CPPFLAGS += -DGRASSROOTS_SERVICE_MANAGER_LIBRARY_EXPORTS 

LDFLAGS = -lpthread \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_USERS_LIB) -l$(GRASSROOTS_USERS_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UUID_LIB) -l$(GRASSROOTS_UUID_LIB_NAME) \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_UUID_LIB) -luuid \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_HANDLER_LIB) -l$(GRASSROOTS_HANDLER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_MONGODB_LIB) -l$(GRASSROOTS_MONGODB_LIB_NAME) \
	-L$(DIR_GRASSROOTS_PLUGIN_LIB) -l$(GRASSROOTS_PLUGIN_LIB_NAME) \
	-L$(DIR_BSON_LIB) -lbson-1.0 \
	-lcurl
	

add_drmaa = 0
ifeq ($(SLURM_DRMAA_ENABLED), 1)
    add_drmaa = 1
endif
ifeq ($(HTCONDOR_DRMAA_ENABLED), 1)
    add_drmaa = 1
endif
ifeq ($(LSF_DRMAA_ENABLED), 1)
    add_drmaa = 1
endif


ifeq ($(add_drmaa),1)
SRCS += drmaa_util.c
LDFLAGS += -L$(DIR_GRASSROOTS_DRMAA_LIB) -l$(GRASSROOTS_DRMAA_LIB_NAME)
INCLUDES += -I$(DIR_GRASSROOTS_DRMAA_INC)
CPPFLAGS += -DDRMAA_ENABLED=1
endif


ifeq ($(IRODS_ENABLED),1)
LDFLAGS += -L$(DIR_GRASSROOTS_IRODS_LIB) -l$(GRASSROOTS_IRODS_LIB_NAME)
INCLUDES += -I$(DIR_GRASSROOTS_IRODS_INC) -I$(DIR_IRODS_INC)
CPPFLAGS += -DIRODS_ENABLED=1
endif



#include $(DIR_ROOT)/irods.mk

#ifeq ($(IRODS_ENABLED), 1)
#LDFLAGS += -L$(DIR_GRASSROOTS_IRODS_LIB) -l$(GRASSROOTS_IRODS_LIB_NAME) $(IRODS_EXTRA_LIBS)
#endif

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile	

