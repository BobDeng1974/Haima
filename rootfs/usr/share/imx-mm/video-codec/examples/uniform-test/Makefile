#################################################################################
# makefile template
# lines with # suffix need to be modified for each instance
# with object file directory
#################################################################################

include ../../../build/Makefile_test.init

#################config information
                     #pc, arm
PLATFORM = arm
                     #yes, no
NEED_GDB = no
                     #exe, sharelib, staticlib, reentry, app
BUILD_ = app

#################directory settings
                     #-I./mod1 -I./mod2
INCDIR = -I./hdr -I../dut_hdr
                     #source directories
SRCDIR0 = ./src/
SRCDIR1 = #./wrapper/c
SRCDIR2 =
                     #share object directory: -L/home/chenfei/work/myshareobject
SODIR = -lm
BINDIR = ../bin
OBJDIR = obj
LIB_DIR=

#CPYDIR = /nfsroot/larry/ROOTFS/home/DecTester

################# board
ifeq ($(MX27),1)
	CPL_DEFINE=-DFSL_MX27
else
	CPL_DEFINE=-DFSL_MX31
endif
#################target related
                     #target name
NAME = test_dec_arm_elinux
SUFFIX =

ifeq ($(BUILD_),app)
  ifeq ($(TGT_CPU),ARM9)
    BIN =    $(BINDIR)/$(NAME)$(SUFFIX)_arm9
  endif
  ifeq ($(TGT_CPU),ARM11)
    BIN =    $(BINDIR)/$(NAME)$(SUFFIX)_arm11
  endif
  ifeq ($(TGT_CPU),ARM12)
    BIN =    $(BINDIR)/$(NAME)$(SUFFIX)_arm12
  endif
endif
#################program compile options
                     #-DDBG=1
CPL_DEFINE += -D__OPTIMISE_TIME --debug  -O0 -DADS -D_ARM11_=1 -DUSE_ASM=1 -DOS_VRTX  -DVRTXMC -DDPB_FIX -D_FSL_VTS# -DTIME_PROFILE
ifeq ($(PLATFORM),arm)
CPL_DEFINE += -DTGT_CPU_ARM11
endif
ifeq ($(PLATFORM),pc)
CPL_DEFINE += -DTGT_CPU_X86
endif

#################compiler options
                     #//-I$(INCDIR)
FLAGS = $(INCDIR)
FLAGS +=  -ffloat-store -Wall -D__USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64
FLAGS += -w      #suppress warnings
FLAGS += -O3     #optimization level
ifeq ($(NEED_GDB),yes)
FLAGS += -g
endif

LIBS = $(SODIR) -lm -lstdc++ -lc -lrt -ldl

#################compile tools
#ifeq ($(PLATFORM),arm)
 # TOOLS_DIR = /opt/montavista/mobilinux/devkit/arm/v6_vfp_le/bin
  #CC = $(TOOLS_DIR)/arm_v6_vfp_le-gcc
 # AS = $(TOOLS_DIR)/arm_v6_vfp_le-as
 # LD = $(TOOLS_DIR)/arm_v6_vfp_le-ld
 # AR = $(TOOLS_DIR)/arm_v6_vfp_le-ar
  #TOOLS_DIR   = /opt/freescale/usr/local/gcc-4.1.1-glibc-2.4-nptl-6/arm-none-linux-gnueabi
  #TOOLS_DIR   = /opt/freescale/usr/local/gcc-4.1.1-glibc-2.4-nptl-3/arm-none-linux-gnueabi
#  TOOLS_DIR   = /vobs/fsl_mad_multimedia_tools/devtek/gcc-4.1.1-glibc-2.4-nptl-6/arm-none-linux-gnueabi
#  CC          = $(TOOLS_DIR)/bin/arm-none-linux-gnueabi-gcc
#  LN          = $(TOOLS_DIR)/bin/arm-none-linux-gnueabi-gcc
#else
#  CC = gcc
#  AS = as
#  LD = ld
#  AR = ar
#endif

#################generate objectives
SRC0 =   $(wildcard $(SRCDIR0)/test_dec_arm_elinux.c) $(wildcard $(SRCDIR0)/render_lcd.c)
SRC1 =   $(wildcard $(SRCDIR1)/*.c)
SRC2 =   $(wildcard $(SRCDIR2)/*.c)
OBJ =    $(SRC0:$(SRCDIR0)/%.c=$(OBJDIR)/%.o$(SUFFIX)) $(SRC1:$(SRCDIR1)/%.c=$(OBJDIR)/%.o$(SUFFIX))  \
         $(SRC2:$(SRCDIR2)/%.c=$(OBJDIR)/%.o$(SUFFIX))

ifeq ($(FLOAT),HW)
  INCDIR+= -I$(SYS_INCLUDE)
  FLAGS += -ftree-vectorize -ffast-math -fsingle-precision-constant -mfloat-abi=hard -DFHW
  LIBS += --sysroot=/usr/armv7-poky-gnueabi-4.7.2-tool-chain/imx6qsabresd
  BIN := $(BIN)_fhw
endif

#########################  targets start here  ##########################################
all: makedir bin copylib

debug:
	@echo '"$(OBJ)"'

makedir:
	@echo
	@echo "Create directory..."
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	@echo

clean:
	@echo remove all objects and bin file
	@rm -f $(OBJDIR)/*
	@rm -f $(BIN)

bin:    $(OBJ)
	@echo

ifeq ($(BUILD_), app)
	@echo 'creating exe binary "$(BIN)"'
	@$(CC) -o $(BIN) $(OBJ) $(LIBS)
endif

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR0)/%.c
	@echo 'compiling object file "$(CC) $@" ...'
	@$(CC) -c -o $@ $(FLAGS) $(CPL_DEFINE) $<

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR1)/%.c
	@echo 'compiling object file "$(CC) $@" ...'
	@$(CC) -c -o $@ $(FLAGS) $(CPL_DEFINE) $<

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR2)/%.c
	@echo 'compiling object file "$(CC) $@" ...'
	@$(CC) -c -o $@ $(FLAGS) $(CPL_DEFINE) $<

copylib:
	@echo '... done'
	@echo

