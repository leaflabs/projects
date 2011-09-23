.DEFAULT_GOAL := all


d := $(abspath .)

# PyMite Configuration
PLATFORM := $(notdir $(CURDIR))
PM_LIB_ROOT := pmvm_$(PLATFORM)
PM_LIB_FN = lib$(PM_LIB_ROOT).a
PM_LIB_PATH := ../../vm/$(PM_LIB_FN)
PM_USR_SOURCES = main.py maple.py 
PMIMGCREATOR := ../../tools/pmImgCreator.py
PMGENPMFEATURES := ../../tools/pmGenPmFeatures.py
BUILD_PATH := build

PMLDARGS :=

IPM=true


#IPM Defines
SERIAL_DEVICE ?= /dev/ttyUSB0
SERIAL_BAUD ?= 115200


PM_INC := -I../../vm
CINCS = $(PM_INC) -I$(abspath .) 

TARGET = main
BUILDDIRS :=
TGT_BIN   :=

#targets
cSRCS :=  $(TARGET)_img.c $(TARGET)_nat.c syscalls.c

cppSRCS := main.cpp plat.cpp pymite_debug.cpp

#libSRCS_$(d) := libpmvm_$(PLATFORM).a 

#cFILES := $(cSRCS:%=$(d)/%)
#cppFILES := $(cppSRCS:%=$(d)/%)
cFILES := $(cSRCS)
cppFILES := $(cppSRCS)
#libFILES_$(d) := $(libSRCS_$(d):%=$(d)/%)

OBJS := $(cFILES:%.c=$(BUILD_PATH)/%.o) \
		$(cppFILES:%.cpp=$(BUILD_PATH)/%.o)

DEPS := $(OBJS:%.o=%.d)

TGT_BIN += $(OBJS) $(BUILD_PATH)/pmstdlib_img.o $(BUILD_PATH)/pmstdlib_nat.o

# Useful tools
CC       := arm-none-eabi-gcc
CXX      := arm-none-eabi-g++
LD       := arm-none-eabi-ld -v
AR       := arm-none-eabi-ar
AS       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
DISAS    := arm-none-eabi-objdump
OBJDUMP  := arm-none-eabi-objdump
SIZE     := arm-none-eabi-size
DFU      := dfu-util


# Suppress annoying output unless V is set
ifndef V
   SILENT_CC       = @echo '  [CC]       ' $(@:$(BUILD_PATH)/%.o=%.c);
   SILENT_AS       = @echo '  [AS]       ' $(@:$(BUILD_PATH)/%.o=%.S);
   SILENT_CXX      = @echo '  [CXX]      ' $(@:$(BUILD_PATH)/%.o=%.cpp);
   SILENT_LD       = @echo '  [LD]       ' $(@F);
   SILENT_AR       = @echo '  [AR]       '
   SILENT_OBJCOPY  = @echo '  [OBJCOPY]  ' $(@F);
   SILENT_DISAS    = @echo '  [DISAS]    ' $(@:$(BUILD_PATH)/%.bin=%).disas;
   SILENT_OBJDUMP  = @echo '  [OBJDUMP]  ' $(OBJDUMP);
endif


# Valid BOARDs: maple, maple_native, ...
BOARD ?= maple_native
MEMORY_TARGET ?= flash

# USB ID for DFU upload
VENDOR_ID  := 1EAF
PRODUCT_ID := 0003

MCU := STM32F103ZE
PRODUCT_ID := 0003
ERROR_LED_PORT := GPIOC
ERROR_LED_PIN  := 15
DENSITY := STM32_HIGH_DENSITY

# Useful paths
ifeq ($(LIB_MAPLE_HOME),)
SRCROOT := .
else
SRCROOT := $(LIB_MAPLE_HOME)
endif
BUILD_PATH = build
LIBMAPLE_PATH := $(SRCROOT)/libmaple
SUPPORT_PATH := $(SRCROOT)/support
#OPENOCD_WRAPPER := $(SRCROOT)/scripts/openocd-wrapper.sh

OBJS += $(CSRCS:%.c=$(BUILD_PATH)/%.o) 
OBJS += $(CPPSRCS:%.cpp=$(BUILD_PATH)/%.o)


# Some target specific things
ifeq ($(MEMORY_TARGET), ram)
   LDSCRIPT := $(BOARD)/ram.ld
   VECT_BASE_ADDR := VECT_TAB_RAM
endif
ifeq ($(MEMORY_TARGET), flash)
   LDSCRIPT := $(BOARD)/flash.ld
   VECT_BASE_ADDR := VECT_TAB_FLASH
endif
ifeq ($(MEMORY_TARGET), jtag)
   LDSCRIPT := $(BOARD)/jtag.ld
   VECT_BASE_ADDR := VECT_TAB_BASE
endif

LIBMAPLE_INCLUDES := -I$(SRCROOT)/libmaple
WIRISH_INCLUDES := -I$(SRCROOT)/wirish -I$(SRCROOT)/wirish/boards -I$(SRCROOT)/wirish/comm


# Compilation flags.
# FIXME remove the ERROR_LED config
GLOBAL_FLAGS    := -D$(VECT_BASE_ADDR)					     \
		   -DBOARD_$(BOARD) -DMCU_$(MCU)			     \
		   -DERROR_LED_PORT=$(ERROR_LED_PORT)			     \
		   -DERROR_LED_PIN=$(ERROR_LED_PIN)			     \
		   -D$(DENSITY) $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(CINCS)

GLOBAL_CFLAGS   := -Os -g3 -gdwarf-2  -mcpu=cortex-m3 -mthumb -march=armv7-m \
		   -nostdlib -ffunction-sections -fdata-sections	     \
		   -Wl,--gc-sections $(GLOBAL_FLAGS) \
		   -D__DEBUG__=1 
GLOBAL_CXXFLAGS := -fno-rtti -fno-exceptions -Wall $(GLOBAL_FLAGS)
GLOBAL_ASFLAGS  := -mcpu=cortex-m3 -march=armv7-m -mthumb		     \
		   -x assembler-with-cpp $(GLOBAL_FLAGS)

LDDIR    := $(SUPPORT_PATH)/ld
LDFLAGS  = -T$(LDDIR)/$(LDSCRIPT) -L$(LDDIR)    \
            -mcpu=cortex-m3 -mthumb -Xlinker     \
            --gc-sections --print-gc-sections --march=armv7-m -Wall \
			-L../../vm -l$(PM_LIB_ROOT) -L.




CFLAGS   = $(GLOBAL_CFLAGS)
CXXFLAGS = $(GLOBAL_CXXFLAGS)
ASFLAGS  = $(GLOBAL_ASFLAGS) 
#LDFLAGS  = $(GLOBA_LDFLAGS)


# Set all submodules here
#LIBMAPLE_MODULES += /home/cospan/Projects/python-on-a-chip/src/platform/maple

# call each module rules.mk
$(foreach m,$(LIBMAPLE_MODULES),$(eval $(call LIBMAPLE_MODULE_template,$(m))))

# Main target
#include build-targets.mk

.PHONY: install all clean help debug cscope tags ctags ram flash jtag doxygen mrproper

# Target upload commands
UPLOAD_ram   := $(SUPPORT_PATH)/scripts/reset.py && \
                sleep 1                  && \
                $(DFU) -a0 -d $(VENDOR_ID):$(PRODUCT_ID) -D $(BUILD_PATH)/$(BOARD).bin -R
UPLOAD_flash := $(SUPPORT_PATH)/scripts/reset.py && \
                sleep 1                  && \
                $(DFU) -a1 -d $(VENDOR_ID):$(PRODUCT_ID) -D $(BUILD_PATH)/$(BOARD).bin -R
UPLOAD_jtag  := support/scripts/openocd-wrapper.sh flash

# conditionally upload to whatever the last build was
install: INSTALL_TARGET = $(shell cat $(BUILD_PATH)/build-type 2>/dev/null)
install: $(BUILD_PATH)/$(BOARD).bin
	@echo Install target: $(INSTALL_TARGET)
	$(UPLOAD_$(INSTALL_TARGET))

# Force a rebuild if the maple target changed
PREV_BUILD_TYPE = $(shell cat $(BUILD_PATH)/build-type 2>/dev/null)
build-check:
ifneq ($(PREV_BUILD_TYPE), $(MEMORY_TARGET))
	$(shell rm -rf $(BUILD_PATH))
endif


#pmvm makefile

all: $(PM_LIB_PATH) build-check MSG_INFO $(BUILD_PATH)/$(BOARD).bin MAKE_BUILD_FOLDER 

.PHONY: MAKE_BUILD_FOLDER

test: MAKE_BUILD_FOLDER
	@echo "Objects: " $(TGT_BIN)
	@echo "Build_Path: " $(BUILD_PATH)
	@echo "Making build folder"

MAKE_BUILD_FOLDER:
	@mkdir -p $(BUILD_PATH)


pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
	$(PMGENPMFEATURES) pmfeatures.py > $@ 

# Generate native code and module images from the python source
$(TARGET)_nat.c $(TARGET)_img.c: $(PM_USR_SOURCES) $(PMIMGCREATOR) pmfeatures.py
	$(PMIMGCREATOR) -f pmfeatures.py -c -u -o $(TARGET)_img.c --native-file=$(TARGET)_nat.c $(PM_USR_SOURCES)


$(PM_LIB_PATH) : ../../vm/*.c ../../vm/*.h pmfeatures.h
	@echo "making library"
	make -C ../../vm


$(BUILD_PATH)/pmstdlib_img.o:../../vm/pmstdlib_img.c MAKE_BUILD_FOLDER
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

$(BUILD_PATH)/pmstdlib_nat.o:../../vm/pmstdlib_nat.c MAKE_BUILD_FOLDER
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

../../vm/pmstdlib_img.c ../../vm/pmstdlib_nat.c :
	$(MAKE) -C ../../vm pmstdlib_img.c pmstdlib_nat.c


$(BUILDDIRS):
	@mkdir -p $@


# General directory independent build rules, generate dependency information
$(BUILD_PATH)/%.o: %.c MAKE_BUILD_FOLDER
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<


$(BUILD_PATH)/%.o: %.cpp MAKE_BUILD_FOLDER
	$(SILENT_CXX) $(CXX) $(CFLAGS) $(CXXFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<


$(BUILD_PATH)/%.o: %.S MAKE_BUILD_FOLDER
	$(SILENT_AS) $(AS) $(ASFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<


clean:
	rm -rf build
	make -C ../../vm clean
	rm $(TARGET)_img.* $(TARGET)_nat.* pmfeatures.h 

cscope:
	rm -rf *.cscope
	find . -name '*.[hcS]' -o -name '*.cpp' | xargs cscope -b

tags:
	etags `find . -name "*.c" -o -name "*.cpp" -o -name "*.h"`
	@echo "Made TAGS file for EMACS code browsing"

ctags:
	ctags-exuberant -R .
	@echo "Made tags file for VIM code browsing"

ram:
	@$(MAKE) MEMORY_TARGET=ram --no-print-directory all

flash:
	@$(MAKE) MEMORY_TARGET=flash --no-print-directory all

jtag:
	@$(MAKE) MEMORY_TARGET=jtag --no-print-directory all

doxygen:
	doxygen $(SUPPORT_PATH)/doxygen/Doxyfile

MSG_INFO:
	@echo "================================================================================"
	@echo ""
	@echo "  Build info:"
	@echo "     BOARD:          " $(BOARD)
	@echo "     MCU:            " $(MCU)
	@echo "     MEMORY_TARGET:  " $(MEMORY_TARGET)
	@echo "     LIB_MAPLE_HOME: " $(LIB_MAPLE_HOME)
	@echo ""
	@echo "  See 'make help' for all possible targets"
	@echo ""
	@echo "================================================================================"
	@echo

$(BUILD_PATH)/$(BOARD).elf: $(BUILDDIRS) $(TGT_BIN)
	$(SILENT_LD) $(CXX) $(LDFLAGS) -o $@ $(TGT_BIN) -L../../vm -lpmvm_$(PLATFORM) -L. -lmaple -Wl,-Map,$(BUILD_PATH)/$(BOARD).map


$(BUILD_PATH)/$(BOARD).bin: $(BUILD_PATH)/$(BOARD).elf
	$(SILENT_OBJCOPY) $(OBJCOPY) -v -Obinary $(BUILD_PATH)/$(BOARD).elf $@ 1>/dev/null
	$(SILENT_DISAS) $(DISAS) -d $(BUILD_PATH)/$(BOARD).elf > $(BUILD_PATH)/$(BOARD).disas
	@echo " "
	@echo "Object file sizes:"
	@find $(BUILD_PATH) -iname *.o | xargs $(SIZE) -t > $(BUILD_PATH)/$(BOARD).sizes
	@cat $(BUILD_PATH)/$(BOARD).sizes
	@echo " "
	@echo "Final Size:"
	@$(SIZE) $<
	@echo $(MEMORY_TARGET) > $(BUILD_PATH)/build-type


IPM:
	../../tools/ipm.py -f pmfeatures.py --serial=$(SERIAL_DEVICE) --baud=$(SERIAL_BAUD)

ALL_CFLAGS := $(GLOBAL_CFLAGS)
export PM_LIB_FN CC OBJCOPY NM ALL_CFLAGS AR LDFLAGS PM_FLAGS
export IPM PLATFORM
