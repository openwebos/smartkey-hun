# Device makefile
BUILD_TYPE := release
PLATFORM   ?= arm

INCLUDES := \
	-Isystem $(INCLUDE_DIR)

ifeq ($(TARGET_ARCH),arm)
LIBS := -L$(LIB_DIR) -llunaservice
FLAGS_OPT += -DTARGET_DEVICE
else
FLAGS_OPT += -DTARGET_EMULATOR
LIBS := -L$(LIB_DIR) -llunaservice
endif

include Makefile.inc
