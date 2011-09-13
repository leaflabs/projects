# Standard things
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)
BUILDDIRS += $(BUILD_PATH)/$(d)

#$(shell python $(PMGENPMFEATURES) pmfeatures.py pmfeatures.h)
#libpmvm_$(PLATFORM).d : libpmvm_$(PLATFORM).a 
#	@echo "HI!"
#	make -C pmvm/vm

# Local flags
CXXFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES)
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) 
#LDFLAGS_$(d) := $(PM_FLAGS) -lpmvm_$(PLATFORM) $(PM_LIB_PATH)


# Local rules and targets
cSRCS_$(d) := 	plat.c 

cppSRCS_$(d) := pmvm.cpp

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


.phony: pmvm


# Standard things
-include $(DEPS_$(d))
d := $(dirstack_$(sp))
sp := $(basename $(sp))
