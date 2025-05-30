NAME := grassroots_util
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
	$(DIR_SRC)/containers \
	$(DIR_SRC)/io \
	$(DIR_SRC)/platform \
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_INCLUDE)/containers \
	-I$(DIR_INCLUDE)/io \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_PCRE2_INC) \
	-I$(DIR_BSON_INC) \
	-I$(DIR_MONGODB_INC)
	
	
ifeq ($(IRODS_ENABLED),1)
INCLUDES += \
	-I$(DIR_SHARED_IRODS_INC)/lib/api/include \
	-I$(DIR_SHARED_IRODS_INC)/lib/core/include \
	-I$(DIR_SHARED_IRODS_INC)/lib/md5/include \
	-I$(DIR_SHARED_IRODS_INC)/lib/sha1/include \
	-I$(DIR_SHARED_IRODS_INC)/server/core/include \
	-I$(DIR_SHARED_IRODS_INC)/server/drivers/include \
	-I$(DIR_SHARED_IRODS_INC)/server/icat/include \
	-I$(DIR_SHARED_IRODS_INC)/server/re/include 
endif	
	
SRCS := \
	byte_buffer.c \
	file_output_stream.c \
	filesystem_utils.c \
	hash_table.c \
	int_linked_list.c \
	json_util.c \
	linked_list.c \
	linked_list_iterator.c \
	math_utils.c \
	operation.c \
	regular_expressions.c \
	resource.c \
	schema_keys.c \
	schema_version.c \
	search_options.c \
	statistics.c \
	streams.c \
	string_hash_table.c \
	string_int_pair.c \
	string_linked_list.c \
	string_utils.c \
	time_util.c 
#	unix_filesystem.c \
#	unix_shared_memory.c \
#	alloc_failure.cpp \

SRCS += $(PLATFORM_SRCS)

ifeq ($(BUILD),release)
	CFLAGS 	+= -O3 -s
 	LDFLAGS += -s
else
	CFLAGS 	+= -g
	CPPFLAGS += -D_DEBUG
endif
	

CPPFLAGS += -DGRASSROOTS_UTIL_LIBRARY_EXPORTS -D$(PLATFORM) 

LDFLAGS += -ldl \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_UUID_LIB) -luuid \
	-L$(DIR_PCRE2_LIB) -l$(PCRE2_LIB_NAME)

HEADERS := $(patsubst %.c, %.h, $(SRCS))
SWIG_HEADERS_TO_IGNORE := \
	alloc_failure.cpp \
	resource.h \

SWIG_HEADERS := $(filter-out $(SWIG_HEADERS_TO_IGNORE),$(HEADERS))

SWIG := /home/billy/Applications/swig/bin/swig

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

.PHONY:	util all test info swig-interface

util: all

test:
	$(COMP) $(CFLAGS) $(CPPFLAGS) -Wl,--no-as-needed -L$(DIR_OBJS)/ -l$(NAME) -lm  $(INCLUDES)  test.c -o test


show-config: 
	@echo "DIR_BUILD: $(DIR_BUILD)"
	@echo "DIR_SRC: $(DIR_SRC)"
	@echo "DIR_INCLUDE: $(DIR_INCLUDE)"
	@echo "DIR_BUILD_CONFIG: $(DIR_BUILD_CONFIG)"

swig-interface:
	@echo "%module util" > util.i
	@echo "%{" >> util.i
	@echo "/* Includes the headers in the wrapper code */" >> util.i
	@$(foreach HEADER,$(SWIG_HEADERS), \
   	echo '#include "$(HEADER)"' >> util.i ; )
	@echo "%}" >> util.i
	@echo "\n" >> util.i
	@echo "/* Parse the headers to generate the wrappers */" >> util.i
	@$(foreach HEADER,$(SWIG_HEADERS), \
   	echo '%include "$(HEADER)"' >> util.i ; )
   	
swig-python:
	$(SWIG) -python $(INCLUDES) $(CPPFLAGS) util.i
