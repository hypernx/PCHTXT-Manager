# devkitPro / libnx Makefile
TARGET := PCHTXT-Manager
BUILD := build
SOURCES := source
INCLUDES := include
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OBJS := $(addprefix $(BUILD)/,$(CFILES:.c=.o))
VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fdata-sections -I$(INCLUDES) -D__SWITCH__
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,--gc-sections
LIBS := -lnx
ifeq ($(strip $(DEVKITPRO)),)
$(error Please set DEVKITPRO in your environment)
endif
include $(DEVKITPRO)/libnx/switch_rules
