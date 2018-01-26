BASE_LIBNAME := grassroots_drmaa
NAME 		:= $(BASE_LIBNAME)_$(DRMAA_IMPLEMENTATION_NAME)
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)


ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(DIR_BUILD)/../../../../../build-config/linux)
endif



include $(DIR_BUILD_CONFIG)/project.properties

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib/drmaa/$(DRMAA_IMPLEMENTATION_NAME)

TEST_EXE_NAME = drmaa_tool_test_$(DRMAA_IMPLEMENTATION_NAME)


BUILD		= debug


VPATH	= \
	$(DIR_SRC) 
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_PARAMS_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_DRMAA_IMPLEMENTATION_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_JANSSON_INC)
	
	
SRCS 	= \
	drmaa_tool.c 

BASE_LDFLAGS += -ldl \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVICES_LIB) -l$(GRASSROOTS_SERVICES_LIB_NAME) \
	-L$(DIR_GRASSROOTS_PARAMS_LIB) -l$(GRASSROOTS_PARAMS_LIB_NAME) \
	-L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_DRMAA_IMPLEMENTATION_LIB) -l$(DRMAA_IMPLEMENTATION_LIB_NAME)

CPPFLAGS += -DGRASSROOTS_DRMAA_LIBRARY_EXPORTS -D$(DRMAA_DEFS)=1
LDFLAGS += $(BASE_LDFLAGS)

EXE_SRCS = $(DIR_SRC)/drmaa_tool_test.c
EXE_OBJS = $(DIR_BUILD)/$(BUILD)/drmaa_tool_test.o

EXE_LDFLAGS = \
	-L$(DIR_GRASSROOTS_SERVICES_LIB) -l$(GRASSROOTS_SERVICES_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVER_LIB) -l$(GRASSROOTS_SERVER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_HANDLER_LIB) -l$(GRASSROOTS_HANDLER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_IRODS_LIB) -l$(GRASSROOTS_IRODS_LIB_NAME) \
	-L$(DIR_GRASSROOTS_MONGODB_LIB) -l$(GRASSROOTS_MONGODB_LIB_NAME) \
	-L$(DIR_GRASSROOTS_DRMAA_LIB) -l$(GRASSROOTS_DRMAA_LIB_NAME) \
	-L$(DIR_HTMLCXX_LIB) -lhtmlcxx \
	-L$(DIR_HCXSELECT_LIB) -lhcxselect \
	-L$(DIR_IRODS_LIB) 	-lirods_client -lirods_common -lirods_plugin_dependencies \
	-L$(DIR_UUID_LIB) -luuid \
	-L$(DIR_PCRE_LIB) -lpcre \
	-L$(DIR_MONGODB_LIB) -lmongoc-1.0 \
	-L$(DIR_BSON_LIB) -lbson-1.0 \


EXE_INCLUDES = \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC)  \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_GRASSROOTS_HANDLER_INC)


ifeq ($(SLURM_DRMAA_ENABLED),1)
CPPFLAGS += -DSLURM_DRMAA_ENABLED
endif

ifeq ($(LSF_DRMAA_ENABLED),1)
CPPFLAGS += -DLSF_DRMAA_ENABLED
endif

ifeq ($(CONDOR_DRMAA_ENABLED),1)
CPPFLAGS += -DCONDOR_DRMAA_ENABLED
endif


include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

.PHONY:	test test_install clean make_install_symbolic_link make_all_symbolic_link lib_install lib_all

ifeq ($(DRMAA_IMPLEMENTATION_NAME),$(DRMAA_DEFAULT_LIB))
install: lib_install make_install_symbolic_link
all: lib_all make_all_symbolic_link
else
install: lib_install
all: lib_all 
endif
	
clean:
	rm -fr $(DIR_OBJS)/*.o

test: 
	@echo "Building DRMAA test tool $(BUILD)/$(TEST_EXE_NAME)"
	gcc $(CPPFLAGS) -Wl,--no-as-needed -I/usr/include $(EXE_INCLUDES) -L$(DIR_OBJS)/ -l$(NAME) -lm $(BASE_LDFLAGS) $(EXE_LDFLAGS) $(INCLUDES) $(EXE_SRCS) -g -o $(BUILD)/$(TEST_EXE_NAME)


test_install: test install
	@echo "Installing $(TEST_EXE_NAME) to $(DIR_GRASSROOTS_INSTALL)"
	cp $(BUILD)/$(TEST_EXE_NAME) $(DIR_GRASSROOTS_INSTALL)/  

lib_all: 


lib_install: all
	@echo "ROOT DIR $(DIR_ROOT)"
	@echo "THIS DIR $(THIS_DIR)"
	@echo "Installing DRMAA library $(TARGET_NAME) to $(DIR_INSTALL)"
	mkdir -p $(DIR_INSTALL)
	cp $(DIR_OBJS)/$(TARGET_NAME) $(DIR_INSTALL)/  
	@echo "checking DRMAA_IMPLEMENTATION_NAME: '$(DRMAA_IMPLEMENTATION_NAME)' against DRMAA_DEFAULT_LIB: '$(DRMAA_DEFAULT_LIB)'"


make_install_symbolic_link:
	@echo "making symbolic link for $(DIR_INSTALL)/$(TARGET_NAME) to $(DIR_GRASSROOTS_INSTALL)/lib/lib$(BASE_LIBNAME).so"
	rm -f  $(DIR_GRASSROOTS_INSTALL)/lib/lib$(BASE_LIBNAME).so
	ln -s $(DIR_INSTALL)/$(TARGET_NAME)  $(DIR_GRASSROOTS_INSTALL)/lib/lib$(BASE_LIBNAME).so

make_all_symbolic_link:
	@echo "making symbolic link for $(DIR_OBJS)/$(TARGET_NAME) to $(DIR_OBJS)/lib$(BASE_LIBNAME).so"
	rm -f $(DIR_OBJS)/lib$(BASE_LIBNAME).so
	ln -s $(DIR_OBJS)/$(TARGET_NAME) $(DIR_OBJS)/lib$(BASE_LIBNAME).so
