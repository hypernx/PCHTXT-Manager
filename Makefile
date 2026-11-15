.SUFFIXES:

include $(DEVKITPRO)/libnx/switch_rules

# Application
TARGET := PCHTXT-Manager
BUILD := build
SOURCES := source
DATA :=
INCLUDES := include

# Compiler and linker options
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fdata-sections
CFLAGS += $(ARCH) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := $(ARCH)
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,--gc-sections
LIBS := -lnx

# Include paths
INCLUDES += $(LIBNX)

