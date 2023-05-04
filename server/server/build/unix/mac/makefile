PLATFORM := mac
CFLAGS += -DMAC

PLATFORM_SRCS := \
	mac_platform.c 

ifeq ($(INCLUDE_TASK), 1)
PLATFORM_SRCS 	+= \
	mac_sync_data.c 
endif

include ../makefile