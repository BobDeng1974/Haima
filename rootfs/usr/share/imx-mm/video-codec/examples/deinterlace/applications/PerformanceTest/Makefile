#/*
# ***********************************************************************
# * Copyright 2005-2007 by Freescale Semiconductor, Inc.
# * All modifications are confidential and proprietary information
# * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
# ***********************************************************************
# */

# Makefile to compile deinterlace application for linux or simulator
#
# History
#   Date           Changed                                      Changed by
#   Sep. 6, 2007   Create                                       Zhenyong Chen
#   Nov. 15, 2007  Refined to unified format                    Zhenyong Chen
#   Jul. 23, 2008  Remove SoC related stuff                     Zhenyong Chen
#*****************************************************************************
#
# Deinterlace application Makefile
#
#*****************************************************************************
CODEC=deinterlace

###############################################################################
# Variables
# ROOT_DIR  - Root of VOB
# BIN_TYPE
# __cplusplus
#
# BUILD     - Toolchain combination
# OPT_LEVEL - Optimization selection
# RVDS_VERSION - Specify when RVDS toolchain is used
# RELEASE   - Release or debug version
# TIME_PROFILE_ENABLE - Enable/disable time profiling

###############################################################################
# Platform specification
# ARM12LERVDS, ARM11LERVDS, ARM11ELINUXNORVDS
BUILD ?= ARM11ELINUXNORVDS

# Tool chain settings according to platform
ROOT_DIR = ../../../..
BIN_TYPE = EXE
# This project contains c++ syntax
__cplusplus = 1
# Release or debug version
RELEASE ?= 1

include Makefile.test.def.init

###############################################################################
# Features support
TIME_PROFILE_ENABLE ?= 1

###############################################################################
# Macros and supplemental compile options
NONE_OS = 0
# For Cortex, SoC
ifeq ($(TGT_OS),LERVDS)
    TIME_PROFILE_ENABLE = 0 # not support
endif # ifeq ($(TGT_OS),LERVDS)

# For elinux
ifeq ($(TGT_OS),ELINUX)
    CDEFS += -D_LINUX
endif # ifeq ($(BUILD),ARM11ELINUXNORVDS)

# Profiling support
ifeq ($(TIME_PROFILE_ENABLE),1)
#  ifeq ($(BUILD),ARM11ELINUXNORVDS)
    CDEFS += -DTIME_PROFILE
#  endif
endif

###############################################################################
# Optimization level
OPT_LEVEL ?= 0
# 0 -- OPT_DEBUG; 1 -- OPT_ARM11; 2 -- OPT_NEON

OPT_DEBUG  = 0
OPT_ARM11  = 0
OPT_NEON   = 0

ifeq ($(OPT_LEVEL),0)
    OPT_DEBUG = 1
else
  ifeq ($(OPT_LEVEL),1)
     OPT_ARM11 = 1
  else
    ifeq ($(OPT_LEVEL),2)
      OPT_NEON = 1
    endif
  endif
endif


ifeq ($(OPT_DEBUG),0)
  ifeq ($(OPT_ARM11),1)
    USE_ARM11_ASM = 1
  else
    USE_ARM11_ASM = 0
  endif
  ifeq ($(OPT_NEON),1)
    USE_NEON_ASM = 1
  else
    USE_NEON_ASM = 0
  endif
else
    CDEFS += -DOPT_DEBUG
    USE_ARM11_ASM = 0
    USE_NEON_ASM = 0
endif

ifeq ($(USE_ARM11_ASM),1)
    CDEFS += -DOPT_ARM11
    ifeq ($(USE_GCC),1)
      AFLAGS += --defsym OPT_ARM11=1
    else
      AFLAGS += --pd "OPT_ARM11 SETL {TRUE}"
    endif
else
    ifeq ($(USE_GCC),1)
      AFLAGS += --defsym OPT_ARM11=0
    else
      AFLAGS += --pd "OPT_ARM11 SETL {FALSE}"
    endif
endif
ifeq ($(USE_NEON_ASM),1)
    CDEFS += -DOPT_NEON
    ifeq ($(USE_GCC),1)
      AFLAGS += --defsym OPT_NEON=1
    else
      AFLAGS += --pd "OPT_NEON SETL {TRUE}"
    endif
else
    ifeq ($(USE_GCC),1)
      AFLAGS += --defsym OPT_NEON=0
    else
      AFLAGS += --pd "OPT_NEON SETL {FALSE}"
    endif
endif


ifeq ($(RELEASE),1)
    CDEFS += -D_RELEASE
else
    CDEFS += -D_DEBUG
endif

###############################################################################
# Derived directories
MW_DIR    = ../../midware
APP_DIR   = .
INCLUDES += -I$(MW_DIR)/hdr

###############################################################################
# Objects
# Put the Cpp files here
CPP_OBJS = $(APP_DIR)/DeintApp.o \
           $(APP_DIR)/RawMovieViewView.o

# Put the C files here
C_OBJS   = 

# Put the asm files here
ASM_OBJS =

# Put midware files here
MW_OBJS =$(MW_DIR)/cpp_src/Deinterlace.o \
         $(MW_DIR)/cpp_src/ProcCtrl.o \
         $(MW_DIR)/cpp_src/PictureManagement.o \
         $(MW_DIR)/cpp_src/RawMovieFile.o

ifdef FULL_RELEASE
MW_OBJS     += $(MW_DIR)/cpp_src/ColorConversion_UnSafe.o
CDEFS      += -DFULL_RELEASE
endif


ALL_OBJS = $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS) $(MW_OBJS)

CFLAGS += $(CDEFS)

###############################################################################
# Targets

.PHONY : help
help:
	@echo make [Options] all
	@echo Options:
	@echo "    BUILD={ARM11ELINUXNORVDS,ARM11LERVDS,ARM12LERVDS} Default to ARM11ELINUXNORVDS"
	@echo "    RVDS_VERSION={2_2,3_0,3_1} Default to 2_2 (2.2). This is used when RVDS is selected for building"
	@echo "    TIME_PROFILE_ENABLE={0,1} Whether time profiling enabled. Default to 1"
	@echo "    RELEASE={0,1} 0 - Debug version; 1 - Release version. Default to 1"
	@echo "    OPT_LEVEL={0,1,2} 0 - c; 1 - Arm11 asm; 2 - Neon asm. Default to 0"
	@echo  

.PHONY : all
all :  $(PROGRAM).axf

$(PROGRAM).axf : $(ALL_OBJS) $(LIBRARY)
	@echo BUILD=$(BUILD) OPT_LEVEL=$(OPT_LEVEL) OPT_DEBUG=$(OPT_DEBUG) OPT_ARM11=$(OPT_ARM11) OPT_NEON=$(OPT_NEON)
	$(LD) -o $@ $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
#	$(FROMELF) --text -s $(PROGRAM) --output info.txt
	@echo "--- Build finished ---"


.PHONY : clean
clean:
	-rm -f $(ALL_OBJS) $(PROGRAM).axf

# Pre- & Post-compile
ifeq ($(BUILD),ARM11ELINUXNORVDS)
PRE_COMPILE_C_CMD = splint -noeffect -likelybool +ptrnegate -boolops -predboolint -aliasunique +matchanyintegral -exportlocal -shiftimplementation $(CDEFS) $(INCLUDES) $<
else
PRE_COMPILE_C_CMD =
endif
POST_COMPILE_C_CMD =

PRE_COMPILE_CPP_CMD = #$(PRE_COMPILE_C_CMD)
POST_COMPILE_CPP_CMD =

PRE_COMPILE_AS_CMD =
POST_COMPILE_AS_CMD =

