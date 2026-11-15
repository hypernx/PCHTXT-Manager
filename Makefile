# PCHTXT-Manager - Nintendo Switch homebrew build
TARGET := PCHTXT-Manager
BUILD := build
SOURCES := source

DEVKITPRO ?= /opt/devkitpro
PREFIX := aarch64-none-elf-
CC := $(PREFIX)gcc

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fdata-sections $(ARCH)
CFLAGS += -D__SWITCH__ -I$(DEVKITPRO)/libnx/include

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH)
LDFLAGS += -Wl,--gc-sections -L$(DEVKITPRO)/libnx/lib
LIBS := -lnx

SOURCES_C := $(wildcard $(SOURCES)/*.c)
OBJECTS := $(patsubst $(SOURCES)/%.c,$(BUILD)/%.o,$(SOURCES_C))

.PHONY: all clean
all: $(TARGET).nro

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(TARGET).nro: $(TARGET).elf
	elf2nro $< $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro
