CURRENT_DIR :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(CURRENT_DIR)/../../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

-include drmaa.properties


ifeq ($(SLURM_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += SLURM
endif

ifeq ($(LSF_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += LSF
endif

ifeq ($(HTCONDOR_DRMAA_ENABLED),1)
DRMAA_IMPLEMENTATIONS += HTCONDOR
endif


.PHONY: all clean install

clean:
	make -f library.makefile clean
	
all:
	@echo "DRMAA_IMPLEMENTATIONS '$(DRMAA_IMPLEMENTATIONS)'"
	$(foreach imp,$(DRMAA_IMPLEMENTATIONS), "make -f library.makefile clean -D$(imp)=1\n")
