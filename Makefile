# Nintendo Switch homebrew build using devkitPro and Plutonium.
.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment.")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET := PCHTXT-Manager
BUILD := build
SOURCES := source
INCLUDES := source

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections $(ARCH) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,--gc-sections

LIBDIRS := $(PORTLIBS) $(LIBNX) $(CURDIR)/Plutonium
LIBS := -lpu -lSDL2_mixer -lopusfile -lopus -lmodplug -lmpg123 -lvorbisidec -logg \
        -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 -lEGL -lGLESv2 -lglapi \
        -ldrm_nouveau -lwebp -lpng -ljpeg `sdl2-config --libs` \
        -lfreetype -lharfbuzz -lz -lbz2 -lnx

.PHONY: all clean
all: Plutonium/lib/libpu.a
	$(MAKE) -f $(DEVKITPRO)/libnx/switch_rules TARGET=$(TARGET) BUILD=$(BUILD) SOURCES=$(SOURCES) INCLUDES=$(INCLUDES) LIBDIRS="$(LIBDIRS)" LIBS="$(LIBS)" CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" all

Plutonium/lib/libpu.a:
	$(MAKE) -C Plutonium

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro $(TARGET).nacp
	$(MAKE) -C Plutonium clean
