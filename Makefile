#
# Project configuration
#

# Path locations for firmware Loader, firmware image tools, etc
TOOLSPATH   = .bin
HEX2IMAGE   = $(TOOLSPATH)/hex2k66img
SENDMIDI    = $(TOOLSPATH)/sendmidi
LOADER      = $(TOOLSPATH)/firmwareloader
JLINKLOADER = $(TOOLCHAINPATH)/bin/JLinkExe

TOOLCHAINPATH = /usr/local
DOXYGEN = $(TOOLCHAINPATH)/doxygen/doxygen

# Location of the arm-none-eabi compiler
COMPILERPATH = $(TOOLCHAINPATH)/arm/bin

# Location of project scripts
SCRIPTSPATH = scripts

# Target CPU
CPUDEVICE = MK66FX1M0XXX18

# The name of your project
TARGET = firmware

# Path location of Electra app
APPPATH = apps/demo

# Location of the low-level firmware functionality
BASEPATH = bsp

# Location own and 3rd party libraries
LIBRARYPATH = lib

# Directory to build in
BUILDDIR = build

# Directory with tests
TESTDIR = tests

# Path where the generated documentation will be stored
DOCS = docs

# JLink command files
JLINKSCRIPTPATH = jlink

# Optional compiler parameters
OPTIONS = -Wno-deprecated-declarations

# Bench workaround. Set to 1 on units whose SD card DAT1..DAT3 lines are
# damaged; the SDIO bus then stays in 1-bit mode. Normal builds leave this at 0.
SD_FORCE_1BIT_BUS = 0

# CPU speed
CPU_SPEED = 180000000


#
# Changes to lines below should not be needed
#

# Detect Host system OS
OS := $(shell uname -s)

# Set MIDI CTRL port based on the OS (Linux vs MacOSX)
ifeq ($(OS),Linux)
  CTRL_PORT = "Electra Controller MIDI 3"
else
  CTRL_PORT = "Electra Controller Electra CTRL"
endif


# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -Os -ggdb -mthumb -ffunction-sections -fdata-sections \
      -fshort-enums $(OPTIONS) -nostdlib -MMD -DF_CPU=$(CPU_SPEED) \
			-Isrc -I$(BASEPATH)

# compiler options for C++ only
CXXFLAGS = -std=gnu++17 -felide-constructors -fexceptions \
      -fshort-enums -Wno-reorder

# linker options
LDFLAGS = -Wl,--gc-sections -mthumb

# additional libraries to link
LIBS = -lstdc++ -lm

# compiler options
CPPFLAGS += -D__MK66FX1M0__ -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
		-DARDUINOJSON_ENABLE_STD_STRING -DARDUINOJSON_ENABLE_ARDUINO_STRING=0 \
		-DARDUINO=182 -DTEENSYDUINO # Required to compile Arduino / Teensy libs
CPPFLAGS += -DSD_FORCE_1BIT_BUS=$(SD_FORCE_1BIT_BUS)
LDSCRIPT = $(BASEPATH)/mk66fx1m0.ld
LDFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-use-linker-plugin \
	-T$(LDSCRIPT)


# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

# automatically create lists of the sources and objects
LC_FILES := $(wildcard $(LIBRARYPATH)/*/*.c)
LCPP_FILES := $(wildcard $(LIBRARYPATH)/**/*.cpp) \
				$(wildcard $(LIBRARYPATH)/*/src/FsLib/*.cpp) \
				$(wildcard $(LIBRARYPATH)/*/src/common/*.cpp) \
				$(wildcard $(LIBRARYPATH)/*/src/iostream/*.cpp) \
				$(wildcard $(LIBRARYPATH)/*/src/ExFatLib/*.cpp) \
				$(wildcard $(LIBRARYPATH)/*/src/DigitalIO/*.cpp) \
			  $(wildcard $(LIBRARYPATH)/*/src/FatLib/*.cpp) \
			  $(wildcard $(LIBRARYPATH)/*/src/SdCard/*.cpp) \
			  $(wildcard $(LIBRARYPATH)/*/src/SpiDriver/*.cpp) \
			  $(wildcard $(LIBRARYPATH)/*/src/*.cpp)
TC_FILES := $(wildcard $(BASEPATH)/*.c) \
				$(wildcard $(BASEPATH)/**/*.c)
TCPP_FILES := $(wildcard $(BASEPATH)/*.cpp) \
				$(wildcard $(BASEPATH)/**/*.cpp)
C_FILES := $(wildcard src/*.c)
CPP_FILES := $(wildcard src/*.cpp) \
				$(wildcard src/**/*.cpp)

APPC_FILES := $(wildcard $(APPPATH)/*/*.c) \
				$(wildcard $(APPPATH)/lib/*/*.c)
APPCPP_FILES := $(wildcard $(APPPATH)/*/*.cpp) \
				$(wildcard $(APPPATH)/src/**/*.cpp) \
			  $(wildcard $(APPPATH)/lib/*/*.cpp)

# include paths for libraries
L_INC := $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/FsLib/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/ExFatLib/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/common/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/DigitalIO/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/FatLib/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/SdCard/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/SpiDriver/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/iostream/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(LIBRARYPATH)/**/src/)), -I$(x))

# include paths for the sources
INC := $(foreach x,$(filter %/, $(wildcard src/Hardware/**/)), -I$(x)) \
	$(foreach x,$(filter %/, $(wildcard src/**/)), -I$(x))

# include paths for Electra application
L_APP_INC := $(foreach x,$(filter %/, $(wildcard $(APPPATH)/**/*/)), -I$(x)) \
		 $(foreach x,$(filter %/, $(wildcard $(APPPATH)/**/)), -I$(x)) \
		 -I$(APPPATH)


SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) \
			 $(APPC_FILES:.c=.o) $(APPCPP_FILES:.cpp=.o) \
		   $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) \
			 $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)
OBJS := $(foreach x,$(SOURCES), $(BUILDDIR)/$(x))


all:	image

format:
	@$(SCRIPTSPATH)/format ./src
	@$(SCRIPTSPATH)/format ./demo
	@$(SCRIPTSPATH)/format $(TESTDIR)

pristine:
	@$(SCRIPTSPATH)/ensure-pristine-tree

test:
	make -C $(TESTDIR)

build:	$(BUILDDIR)/$(TARGET).elf

hex:	$(BUILDDIR)/$(TARGET).hex

image:	$(BUILDDIR)/$(TARGET).img

upload: $(BUILDDIR)/$(TARGET).hex
	$(SENDMIDI) dev $(CTRL_PORT) syx 00h 21h 45h 7fh 7fh
	$(LOADER) $<

upload-jlink: $(BUILDDIR)/$(TARGET).hex
	$(JLINKLOADER) -nogui 1 -device $(CPUDEVICE) -CommandFile $(JLINKSCRIPTPATH)/erase.jlink
	$(JLINKLOADER) -nogui 1 -device $(CPUDEVICE) -CommandFile $(JLINKSCRIPTPATH)/download.jlink

docs: $(OBJS)
	@$(DOXYGEN)

$(BUILDDIR)/%.o: %.c
	@echo -e "[CC] \t$<"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CPPFLAGS) $(INC) $(L_INC) $(L_APP_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo -e "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) $(L_INC) $(L_APP_INC) -o "$@" -c "$<"

$(BUILDDIR)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo -e "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS)

$(BUILDDIR)/%.hex:	$(BUILDDIR)/%.elf
	@echo -e "[HEX]\t$@"
	@$(SIZE) "$<"
	@$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

$(BUILDDIR)/%.img:	$(BUILDDIR)/%.hex
	@echo -e "[IMG]\t$@"
	@$(HEX2IMAGE) $< $(basename $<).img

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf "$(BUILDDIR)"
	@rm -f "$(TARGET).elf" "$(TARGET).hex"
	@rm -rf $(DOCS)
	@make -C $(TESTDIR) clean
