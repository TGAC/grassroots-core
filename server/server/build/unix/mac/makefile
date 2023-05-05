PLATFORM := mac
CFLAGS += -DMAC

PLATFORM_SRCS := \
	mac_platform.c 


ifeq ($(BUILD_COMBINED), 1)
PLATFORM_SRCS += \
	mac_sync_data.c \
	mac_async_task.c 
endif

include ../makefile