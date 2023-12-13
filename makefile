export DIR_CORE :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

COPY	:= cp 
DELETE := rm


ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath ../build-config/unix/)
endif

include $(DIR_BUILD_CONFIG)/project.properties

-include drmaa.properties


DIRS :=	$(DIR_GRASSROOTS_UTIL_BUILD) \
	$(DIR_GRASSROOTS_USERS_BUILD) \
	$(DIR_GRASSROOTS_UUID_BUILD) \
	$(DIR_GRASSROOTS_PLUGIN_BUILD) \
	$(DIR_GRASSROOTS_NETWORK_BUILD) \
	$(DIR_GRASSROOTS_HANDLER_BUILD) \
	$(DIR_GRASSROOTS_MONGODB_BUILD) \

ifeq ($(BUILD_COMBINED), 1)
DIRS += $(DIR_GRASSROOTS_SERVER_BUILD) \
	$(DIR_GRASSROOTS_TASK_BUILD) \
	$(DIR_GRASSROOTS_SERVICES_BUILD) 
else
DIRS += \
	$(DIR_GRASSROOTS_TASK_BUILD) \
	$(DIR_GRASSROOTS_SERVICES_BUILD) 
endif
	
DIRS += \
	$(DIR_GRASSROOTS_CLIENTS_BUILD) \
	$(DIR_GRASSROOTS_SQLITE_BUILD) \
	$(DIR_GRASSROOTS_LUCENE_BUILD) 	


ifneq ($(BUILD_COMBINED), 1)
DIRS += $(DIR_GRASSROOTS_SERVER_BUILD)
endif


ifeq ($(IRODS_ENABLED), 1)
DIRS += $(DIR_GRASSROOTS_IRODS_BUILD)
endif




ifeq ($(SLURM_DRMAA_ENABLED),1)
DIRS += $(DIR_GRASSROOTS_DRMAA_BUILD)
else ifeq ($(LSF_DRMAA_ENABLED),1)
DIRS += $(DIR_GRASSROOTS_DRMAA_BUILD)
else ifeq ($(HTCONDOR_DRMAA_ENABLED),1)
DIRS += $(DIR_GRASSROOTS_DRMAA_BUILD)
endif

	
DIRS +=	 \
	$(DIR_GRASSROOTS_LUCENE_BUILD) 

#
# Make some phony targets to allow us to pass
# targets to the makefiles in each of DIRS
#
DIRS_ALL := $(DIRS:%=all-%)
DIRS_INSTALL := $(DIRS:%=install-%)
DIRS_CLEAN := $(DIRS:%=clean-%)


# targets depends on the same target name in each of the directories 
all: $(DIRS_ALL)
install: $(DIRS_INSTALL)
clean: $(DIRS_CLEAN)

show-config: 
	@echo "DIR_GRASSROOTS_UTIL_BUILD: $(DIR_GRASSROOTS_UTIL_BUILD)"
	@echo "DIR_GRASSROOTS_UUID_BUILD: $(DIR_GRASSROOTS_UUID_BUILD)"


# remove the fake prefix and call the target
$(DIRS_ALL):
	@$(MAKE) $(MAKE_FLAGS) -C $(@:all-%=%) all

$(DIRS_INSTALL):
	@$(MAKE) $(MAKE_FLAGS) -C $(@:install-%=%) install

$(DIRS_CLEAN):
	@$(MAKE) $(MAKE_FLAGS) -C $(@:clean-%=%) clean


.PHONY: subdirs $(DIRS_ALL)
.PHONY: subdirs $(DIRS_INSTAALL)
.PHONY: subdirs $(DIRS_CLEAN)
.PHONY: all install clean 
