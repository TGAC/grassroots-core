NAME 		= grassroots_mongodb
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties
-include $(DIR_BUILD)/user.prefs

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib



# BEGIN MONGODB CONFIGURATION
ifneq ($(MONGODB_HOME),)
DIR_MONGODB := $(MONGODB_HOME)
else
DIR_MONGODB := /usr/local
endif
export DIR_MONGODB_INC := $(DIR_MONGODB)/include/libmongoc-1.0
export DIR_MONGODB_LIB := $(DIR_MONGODB)/lib
# END MONGODB CONFIGURATION


# BEGIN BSON CONFIGURATION
ifneq ($(BSON_HOME),)
DIR_BSON := $(BSON_HOME)
else
DIR_BSON := /usr/local
endif
export DIR_BSON_INC := $(DIR_BSON)/include/libbson-1.0
export DIR_BSON_LIB := $(DIR_BSON)/lib
# END BSON CONFIGURATION



BUILD		:= debug

VPATH	:= \
	$(DIR_SRC) 
	

INCLUDES := \
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

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile


