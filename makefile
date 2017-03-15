DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

COPY	:= cp 
DELETE := rm

#export DIR_SERVER_ROOT_BUILD = $(DIR_SERVER_BUILD_ROOT)/server
#export DIR_SERVER_ROOT_SRC = $(DIR_SERVER_SRC_ROOT)/server

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath ../build_config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

-include drmaa.properties


#ifdef build_drmaa_flag
drmaa: lib_util lib_network lib_parameters lib_irods lib_handler lib_services
	@echo "DRMAA_IMPLEMENTATION_NAME $(DRMAA_IMPLEMENTATION_NAME)"
	@echo "DRMAA_IMPLEMENTATION_LIB_NAME $(DRMAA_IMPLEMENTATION_LIB_NAME)"
	@echo "DIR_DRMAA_IMPLEMENTATION_LIB $(DIR_DRMAA_IMPLEMENTATION_LIB)"
	@echo "DIR_DRMAA_IMPLEMENTATION_INC $(DIR_DRMAA_IMPLEMENTATION_INC)"
#endif


# SLURM DRMAA
ifeq ($(SLURM_DRMAA_ENABLED),1)	
export SLURM_DRMAA_ENABLED := 1
core: drmaa_slurm

install: install_drmaa_slurm

drmaa_slurm: drmaa
	@echo "\n****** BEGIN DRMAA SLURM ******"
	$(MAKE) -C $(DIR_GRASSROOTS_DRMAA_BUILD)
	@echo "****** END DRMAA SLURM ******\n"
	
install_drmaa_slurm: drmaa_slurm
	$(MAKE) -C $(DIR_SERVER_BUILD_ROOT)/drmaa install


clean:
	$(MAKE) -C $(DIR_GRASSROOTS_DRMAA_BUILD) $@
	

# LSF DRMAA
else ifeq ($(LSF_DRMAA_ENABLED),1)
export LSF_DRMAA_ENABLED := 1

core: drmaa_lsf

install: install_drmaa_lsf

drmaa_lsf: drmaa
	@echo "\n****** BEGIN DRMAA LSF ******"
	$(MAKE) -C $(DIR_GRASSROOTS_DRMAA_BUILD)
	@echo "****** END DRMAA LSF ******\n"

install_drmaa_lsf: drmaa_lsf
	$(MAKE) -C $(DIR_SERVER_BUILD_ROOT)/drmaa install 

clean:
	$(MAKE) -C $(DIR_GRASSROOTS_DRMAA_BUILD) $@

endif


DIRS := 	\
	$(DIR_GRASSROOTS_UTIL_BUILD) \
	$(DIR_GRASSROOTS_NETWORK_BUILD) \
	$(DIR_GRASSROOTS_PARAMS_BUILD) \
	$(DIR_GRASSROOTS_HANDLER_BUILD) \
	$(DIR_GRASSROOTS_CLIENTS_BUILD) \
	$(DIR_GRASSROOTS_SERVICES_BUILD) \
	$(DIR_GRASSROOTS_SERVER_BUILD) \
	$(DIR_GRASSROOTS_MONGODB_BUILD) \
#	$(DIR_GRASSROOTS_IRODS_BUILD) \

#
# Make some phony targets to allow us to pass
# targets to the makefiles in each of DIRS
#
DIRS_ALL := $(DIRS:%=all-%)
DIRS_INSTALL := $(DIRS:%=install-%)
DIRS_CLEAN := $(DIRS:%=clean-%)


# targets depends on the same target name in each of the directories 
all: $(DIRS_ALL)
install: $(DIRS_INSTAALL)
clean: $(DIRS_CLEAN)


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
