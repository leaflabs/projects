# Standard things
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)
BUILDDIRS += $(BUILD_PATH)/$(d)

# PyMite Configuration
PLATFORM := $(notdir $(CURDIR))
PM_LIB_ROOT := pmvm_$(PLATFORM)
PM_LIB_FN = lib$(PM_LIB_ROOT).a
PM_LIB_PATH := ../../vm/$(PM_LIB_FN)
PM_USR_SOURCES = main.py maple.py 
PMIMGCREATOR := ../../tools/pmImgCreator.py
PMGENPMFEATURES := ../../tools/pmGenPmFeatures.py

IPM=true

PM_INC := -I../../vm
CINCS = $(PM_INC) -I$(abspath .) 

# Local flags
CXXFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(CINCS)
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(CINCS) 
#LDFLAGS_$(d) := $(PM_FLAGS) -lpmvm_$(PLATFORM) $(PM_LIB_PATH)


# Local rules and targets
cSRCS_$(d) :=  $(TARGET)_img.c $(TARGET)_nat.c syscalls.c ../../pmstdlib_img.c ../../pmstdlib_nat.c

cppSRCS_$(d) := plat.cpp

#libSRCS_$(d) := libpmvm_$(PLATFORM).a 

cFILES_$(d) := $(cSRCS_$(d):%=$(d)/%)
cppFILES_$(d) := $(cppSRCS_$(d):%=$(d)/%)
#libFILES_$(d) := $(libSRCS_$(d):%=$(d)/%)

OBJS_$(d) := $(cFILES_$(d):%.c=$(BUILD_PATH)/%.o) \
             $(cppFILES_$(d):%.cpp=$(BUILD_PATH)/%.o)

DEPS_$(d) := $(OBJS_$(d):%.o=%.d)
#DEPS_$(d) += $(libFILES_$(d): %.a=%.d)
#DEPS_$(d) += $(LOCAL_OBS_%(d): %.c=%.d))

#pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
#	$(PMGENPMFEATURES) pmfeatures.py > $@

$(OBJS_$(d)): TGT_CXXFLAGS := $(CXXFLAGS_$(d))
$(OBJS_$(d)): TGT_CFLAGS := $(CFLAGS_$(d))
#$(OBJS_$(d)): TGT_LDFLAGS := $(LDFLAGS_$(d))



TGT_BIN += $(OBJS_$(d))


#$(TARGET)_img.o : $(d)/$(TARGET)_img.c
#	@echo "building " $(TARGET)_img.o " from " $(TARGET)_img.c
#	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

#$(TARGET)_nat.o : $(TARGET)_nat.c
#	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

#pmvm : $(PM_LIB_PATH)

pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
	$(PMGENPMFEATURES) pmfeatures.py > $@ 

bytecode: $(TARGET)_nat.c $(TARGET)_img.c pmfeatures.h
	make -C ../../vm bytecode PLATFORM=maple 

$(PM_LIB_PATH) : ../../vm/*.c ../../vm/*.h
	make -C ../../vm

# Generate native code and module images from the python source
$(TARGET)_nat.c $(TARGET)_img.c: $(PM_USR_SOURCES) $(PMIMGCREATOR) pmfeatures.py
	$(PMIMGCREATOR) -f pmfeatures.py -c -u -o $(TARGET)_img.c --native-file=$(TARGET)_nat.c $(PM_USR_SOURCES)



build/$(abspath .)/$(TARGET)_img.o:$(TARGET)_img.c
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

build/$(abspath .)/$(TARGET)_nat.o:$(TARGET)_nat.c
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<


build/$(abspath .)/../../pmstdlib_img.o:../../vm/pmstdlib_img.c
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<

build/$(abspath .)/../../pmstdlib_nat.o:../../vm/pmstdlib_nat.c
	$(SILENT_CC) $(CC) $(CFLAGS) -MMD -MP -MF $(@:%.o=%.d) -MT $@ -o $@ -c $<


../../vm/pmstdlib_img.c ../../vm/pmstdlib_nat.c :
	$(MAKE) -C ../../vm pmstdlib_img.c pmstdlib_nat.c



export IPM


# Standard things
-include $(DEPS_$(d))
d := $(dirstack_$(sp))
sp := $(basename $(sp))
