# devkitPro / libnx Makefile
.SUFFIXES:

TARGET := PCHTXT-Manager
BUILD := build
SOURCES := source
DATA :=
INCLUDES := include

# Application metadata
APP_TITLE := PCHTXT Manager
APP_AUTHOR := PCHTXT-Manager
APP_VERSION := 0.1.0

# Compiler options
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fdata-sections
CFLAGS += $(ARCH) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := $(ARCH)
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,--gc-sections
LIBS := -lnx

ifeq ($(strip $(DEVKITPRO)),)
$(error Please set DEVKITPRO in your environment)
endif

include $(DEVKITPRO)/libnx/switch_rules
