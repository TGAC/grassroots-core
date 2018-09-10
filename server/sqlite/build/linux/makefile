NAME 		= grassroots_sqlite
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties
-include $(DIR_BUILD)/user.prefs

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib


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
	-I$(DIR_SQLITE_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_JANSSON_INC) \
	
SRCS 	= \
	sqlite_tool.c \
	sql_clause.c \
	sql_clause_list.c \
	sqlite_column.c
	

BASE_LDFLAGS = -ldl \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_SQLITE_LIB) -lsqlite3 \
	-L$(DIR_JANSSON_LIB) -ljansson
	

CPPFLAGS += -DGRASSROOTS_SQLITE_LIBRARY_EXPORTS

LDFLAGS += $(BASE_LDFLAGS)

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

.PHONY: test_app run_test_app

test_app: install 
	gcc $(DIR_SRC)/sqlite_tool_test.c -o $(BUILD)/sqlite_tool_test $(INCLUDES) $(CFLAGS) -D_DEBUG $(BASE_LDFLAGS) -L$(BUILD) -lgrassroots_sqlite -DUNIX=1 -L$(DIR_PCRE_LIB) -lpcre -lpthread


run_test_app: test_app
	test_envvars && $(BUILD)/sqlite_tool_test
