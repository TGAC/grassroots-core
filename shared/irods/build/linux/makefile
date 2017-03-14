NAME 		= grassroots_irods
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)
MAKEDEPEND 	:= gcc -MM -MF


ifeq ($(DIR_BUILD_UTILS),)
export DIR_BUILD_UTILS := $(realpath $(DIR_BUILD)/../../../../../build_utils/linux)
endif
ifneq ($(PROPERTIES_SET),1)
include $(DIR_ROOT)/project.properties
endif

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/lib

BUILD		= debug


ifeq ($(IRODS_ENABLED),1)

lib_irods:
	$(MAKE) -C $(DIR_GRASSROOTS_IRODS_BUILD)

install_lib_irods:	lib_irods
	$(MAKE) -C $(DIR_GRASSROOTS_IRODS_BUILD) install

handler_irods:
	$(MAKE) -C $(DIR_HANDLER_ROOT_BUILD)/irods

install_handler_irods: handler_irods
	$(MAKE) -C $(DIR_HANDLER_ROOT_BUILD)/irods install

service_irods_search: 
	$(MAKE) -C $(DIR_SERVICES_ROOT)/irods_search

install_service_irods_search: service_irods_search
	$(MAKE) -C $(DIR_SERVICES_ROOT)/irods_search install

#############################
######### END IRODS #########
#############################


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


CFLAGS += -DIRODS_ENABLED=1 


else

lib_irods:	
	echo "**** Skip building lib_irods as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

install_lib_irods:	
	echo "**** Skip running install_lib_irods as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

handler_irods:
	echo "**** Skip building handler_irods as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

install_handler_irods:
	echo "**** Skip running install_handler_irods as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

service_irods_search:
	echo "**** Skip building service_irods_search as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

install_service_irods_search:
	echo "**** Skip running install_service_irods_search as IRODS_ENABLED != 1, $(IRODS_ENABLED)" 

endif		# ifeq ($(IRODS_ENABLED),1)


clean:
	@rm -fr $(BUILD)/

