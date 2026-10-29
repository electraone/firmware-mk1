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

# Path location of Electra app. Three live in apps/: the demo, the
# controller, and the Electra Link satellite program (see apps/link/README.md).
# An app may carry an app.mk that adds sources and include paths of its own -
# the link program borrows the controller's components that way.
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

# Optional compiler parameters
OPTIONS = -Wno-deprecated-declarations

# Bench workaround. Set to 1 on units whose SD card DAT1..DAT3 lines are
# damaged; the SDIO bus then stays in 1-bit mode. Normal builds leave this at 0.
SD_FORCE_1BIT_BUS = 0

# Serial number of the J-Link probe to flash through, for upload-jlink and
# erase-jlink. Leave it empty when a single probe is attached. When more than
# one is, JLinkExe silently takes whichever it enumerates first, so it has to be
# told which one: set this, or export JLINK_SN, to the serial that JLinkExe
# prints for ShowEmuList.
JLINK_SN ?=

# Size of the buffer the behavioural trace writes into. Only allocated when
# the firmware is built with TRACE=1. Must be a power of two: the buffer is
# aligned to its own size so that it cannot straddle the boundary between the
# two SRAM banks, which a single access may not cross.
TRACE_BUFFER_SIZE = 32768

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

# The J-Link invocation shared by the SWD targets. -ExitOnError makes JLinkExe
# return a failure instead of working through the rest of the command file and
# reporting success, which is what hides a flash that never happened.
JLINKCMD = $(JLINKLOADER) -nogui 1 $(if $(JLINK_SN),-SelectEmuBySN $(JLINK_SN),) \
      -device $(CPUDEVICE) -ExitOnError 1


# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -Os -ggdb -mthumb -ffunction-sections -fdata-sections \
      -fshort-enums $(OPTIONS) -nostdlib -MMD -DF_CPU=$(CPU_SPEED) \
			-I. -Isrc -I$(BASEPATH)

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
OBJDUMP = $(abspath $(COMPILERPATH))/arm-none-eabi-objdump

# automatically create lists of the sources and objects
LC_FILES := $(wildcard $(LIBRARYPATH)/*/*.c)

# Display SPI benchmark. Runs once at boot and reports through the logger, so
# it needs TRACE=1 as well to be visible.
ifeq ($(BENCHMARK),1)
  CPPFLAGS += -DELECTRA_BENCHMARK=1
endif

# Behavioural trace for the compatibility harness. Off by default; the SEGGER
# RTT transport is left out of the build entirely unless TRACE=1 is given.
ifeq ($(TRACE),1)
  CPPFLAGS += -DELECTRA_TRACE=1 \
              -DELECTRA_TRACE_BUFFER_SIZE=$(TRACE_BUFFER_SIZE) \
              -DBUFFER_SIZE_UP=64 \
              -DSEGGER_RTT_MAX_NUM_UP_BUFFERS=2 \
              -DSEGGER_RTT_MAX_NUM_DOWN_BUFFERS=1
else
  LC_FILES := $(filter-out $(LIBRARYPATH)/SEGGER_RTT/%,$(LC_FILES))
endif
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

# An application's own additions to the source and include lists.
-include $(APPPATH)/app.mk


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

# Flash over SWD with a J-Link, and let the firmware run. The core is reset and
# halted before the download: connecting on its own leaves the resident firmware
# running, and it then disturbs the RAMCode that does the programming - the
# symptom is "Timeout while erasing sectors" with the reported PC inside flash
# rather than in the RAMCode's SRAM.
#
# LoadFile erases the sectors it writes, so there is no full-chip erase in front
# of it: a blanket "erase" also covers the FlexNVM at 0x10000000, which the K66
# rejects with "Algo35: Illegal command" and which then leaves the RAMCode stuck,
# so the download that follows it cannot connect. Use erase-jlink for a
# deliberate wipe.
#
# The command file is generated rather than kept in the tree because it has to
# name $(BUILDDIR), which varies per build (build-trace, build-prof), and because
# writing a generated file into the tracked tree would break the pristine target.
upload-jlink: $(BUILDDIR)/$(TARGET).hex
	@printf 'SelectInterface SWD\nSpeed 1000\nconnect\nr\nh\nLoadFile $<\nr\ngo\nquit\n' > $(BUILDDIR)/download.jlink
	$(JLINKCMD) -CommandFile $(BUILDDIR)/download.jlink

# Erase the program flash. Stays within bank 0 (1 MB on the K66) so that it does
# not trip over the FlexNVM, which the blanket form of the command does.
erase-jlink:
	@mkdir -p $(BUILDDIR)
	@printf 'SelectInterface SWD\nSpeed 1000\nconnect\nerase 0x0 0xFFFFF\nquit\n' > $(BUILDDIR)/erase.jlink
	$(JLINKCMD) -CommandFile $(BUILDDIR)/erase.jlink

docs: $(OBJS)
	@$(DOXYGEN)

# Object files depend on the compiler flags as well as on the sources. Without
# this, changing a flag such as TRACE or SD_FORCE_1BIT_BUS leaves every object
# stale and links a firmware that is a silent mixture of both settings.
BUILDFLAGS := $(CPPFLAGS) $(CXXFLAGS) $(INC) $(L_INC) $(L_APP_INC)

$(BUILDDIR)/.buildflags: FORCE
	@mkdir -p "$(dir $@)"
	@echo '$(BUILDFLAGS)' | cmp -s - "$@" || echo '$(BUILDFLAGS)' > "$@"

FORCE:

$(BUILDDIR)/%.o: %.c $(BUILDDIR)/.buildflags
	@echo -e "[CC] \t$<"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CPPFLAGS) $(INC) $(L_INC) $(L_APP_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/.buildflags
	@echo -e "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) $(L_INC) $(L_APP_INC) -o "$@" -c "$<"

$(BUILDDIR)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo -e "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS)
	@python3 tools/check-sram-boundary.py $(OBJDUMP) "$@" || { rm -f "$@"; exit 1; }

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
