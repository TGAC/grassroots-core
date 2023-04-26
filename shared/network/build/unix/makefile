NAME := grassroots_network
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/$(PLATFORM))
DIR_SRC := $(realpath $(DIR_BUILD)/../../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../../build-config/unix/)
#export DIR_BUILD_CONFIG = /Users/tyrrells/Projects/grassroots/build-config/unix/
endif

include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL := $(DIR_GRASSROOTS_INSTALL)/lib

TEST_EXE_NAME = network_test

BUILD	:= debug

VPATH	:= \
	$(DIR_SRC) 
	

INCLUDES := \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_UUID_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC)/parameters \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_PLUGIN_INC) \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_PCRE_INC)	
	
SRCS 	:= \
	connection.c \
	curl_tools.c \
	json_tools.c \
	key_value_pair.c \
	raw_connection.c \

	

BASE_LDFLAGS := -ldl \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_UUID_LIB) -l$(GRASSROOTS_UUID_LIB_NAME) \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-lcurl \

	
CPPFLAGS += -DGRASSROOTS_NETWORK_LIBRARY_EXPORTS  -I$(DIR_HTMLCXX_INC)

LDFLAGS += $(BASE_LDFLAGS)


include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

.PHONY:	test test_install network

network: all


test:
	$(COMP)  -Wl,--no-as-needed -I/usr/include -L$(DIR_OBJS)/ -l$(NAME) -lm $(BASE_LDFLAGS) $(INCLUDES)  $(DIR_SRC)/test.c -g -o $(BUILD)/$(TEST_EXE_NAME)


test_install: test install
	@echo "Installing $(TEST_EXE_NAME) to $(DIR_INSTALL_ROOT)"
	cp $(BUILD)/$(TEST_EXE_NAME) $(DIR_INSTALL_ROOT)/  

