PLATFORM := linux
CFLAGS += -DLINUX

PLATFORM_SRCS = \
	linux_platform.c 


ifeq ($(BUILD_COMBINED), 1)
PLATFORM_SRCS += \
	linux_sync_data.c \
	linux_async_task.c 

endif

include ../makefile