CURRENT_DIR :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(CURRENT_DIR)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

-include drmaa.properties


ifeq ($(SLURM_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += slurm
endif

ifeq ($(LSF_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += lsf
endif

ifeq ($(HTCONDOR_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += htcondor
endif


.PHONY: all clean install test

clean:
	make -f library.makefile clean
	
all:
	@echo "DRMAA_IMPLEMENTATIONS '$(DRMAA_IMPLEMENTATIONS)'"
	@$(foreach imp,$(DRMAA_IMPLEMENTATIONS), \
		make -f $(imp).makefile clean all;)

install:
	@$(foreach imp,$(DRMAA_IMPLEMENTATIONS), \
		make -f $(imp).makefile clean install;)
		
test:
	@$(foreach imp,$(DRMAA_IMPLEMENTATIONS), \
		make -f $(imp).makefile test;)

		