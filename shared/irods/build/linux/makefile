NAME 		= grassroots_irods
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)


ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build_utils/linux)
endif
include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

# Add in the appropriate irods libs and dependencies
IRODS_VERSION_MAJOR := $(shell echo $(IRODS_VERSION) | cut -f1 -d ".")
IRODS_VERSION_MINOR := $(shell echo $(IRODS_VERSION) | cut -f2 -d ".")

IRODS_GT_4_2 := $(shell [ $(IRODS_VERSION_MAJOR) -gt 4 -o \( $(IRODS_VERSION_MAJOR) -eq 4 -a $(IRODS_VERSION_MINOR) -ge 2 \) ] && echo true)

ifeq ($(IRODS_GT_4_2), true)
LIBS += \
	irods_client \
	irods_common     \
	irods_plugin_dependencies 
LIBPATHS = \
	$(IRODS_EXTERNALS)/boost1.60.0-0/lib \
	$(IRODS_EXTERNALS)/jansson2.7-0/lib
else
LIBS += = \
	irods_client_core    \
	irods_client_api \
	irods_client_api_table \
	irods_client_plugins 
        
LIBPATHS = \
	      $(IRODS_EXTERNALS)
endif		# ifeq ($(IRODS_GT_4_2), true)




VPATH	= \
	$(DIR_SRC) 

INCLUDES += \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_IRODS_INC)
	
SRCS 	= \
	connect.c \
	meta_search.c \
	query.c \
	user.c \
	irods_utils.c
	
CPPFLAGS += -DUNIX -DIRODS_UTIL_LIBRARY_EXPORTS


CFLAGS += -DIRODS_ENABLED=1 


include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

