#################################################################################
# makefile template
# lines with # suffix need to be modified for each instance
# with object file directory
#################################################################################

CODEC = H264_dec

#################config information
                     #pc, arm
PLATFORM = arm
                     #yes, no
NEED_GDB = no
                     #exe, sharelib, staticlib
BUILD ?= ARM11ELINUX

SUPPORT_OUTPUT_FRAME_EARLY=0
#########################

include ../../../build/Makefile_test.init

#################directory settings
                     #-I./mod1 -I./mod2
INCDIR = -I./hdr/ -I ../dut_hdr -I./../../../ghdr
                     #source directories
SRCDIR0 = ./src/
SRCDIR1 = #./wrapper/c
SRCDIR2 =
                     #share object directory: -L/home/chenfei/work/myshareobject
SODIR = -lm
BINDIR = ../bin
OBJDIR = obj
#LIB_DIR=./../../../src/H264_dec

#CPYDIR = /nfsroot/xianzhong/
#CPYDIR = /nfsroot/larry/ROOTFS/home/DecTester

#################target related
                     #target name
NAME = DutDecH264
SUFFIX =
#ifeq ($(BUILD),exe)
#BIN =    $(BINDIR)/$(NAME)$(SUFFIX).exe
#endif
ifeq ($(BUILD),ARM11ELINUX)
BIN =    $(BINDIR)/lib$(NAME)$(SUFFIX)_arm11.so
endif
#ifeq ($(BUILD),ARM11ELINUXDLIB)
#BIN =    $(BINDIR)/lib$(NAME)$(SUFFIX).so
#endif
ifeq ($(BUILD),ARM9ELINUX)
BIN =    $(BINDIR)/lib$(NAME)$(SUFFIX)_arm9.so
endif
#ifeq ($(BUILD),ARM9ELINUXDLIB)
#BIN =    $(BINDIR)/lib$(NAME)$(SUFFIX).so
#endif
#ifeq ($(BUILD),staticlib)
#BIN =    $(BINDIR)/lib$(NAME)$(SUFFIX).a
#endif

CODEC_LIBRARY=../$(LIBRARY)

#################program compile options
                     #-DDBG=1
CPL_DEFINE = -D__OPTIMISE_TIME --debug  -O0 -DADS -D_ARM11_=1 -DUSE_ASM=1 -DOS_VRTX  -DVRTXMC -DDPB_FIX -D_FSL_VTS# -DTIME_PROFILE
ifeq ($(PLATFORM),arm)
CPL_DEFINE += -DTGT_CPU_ARM11
endif

ifeq ($(SUPPORT_OUTPUT_FRAME_EARLY),1)
CPL_DEFINE += -DSUPPORT_OUTPUT_FRAME_EARLY
endif

ifeq ($(DR_LINUX_TEST),1)
CPL_DEFINE += -DDR_LINUX_TEST
endif

ifeq ($(BUILD),ARM9ELINUX)
CPL_DEFINE += -DSW_DEBLOCK
endif

#ifeq ($(BUILD),ARM9ELINUXDLIB)
#CPL_DEFINE += -DSW_DEBLOCK
#endif

ifeq ($(SW_DEBLOCK),1)
CPL_DEFINE += -DSW_DEBLOCK
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

#################link options
#ifeq ($(BUILD),exe)
                 #libraries: -l, such as -lm
LIBS =
#endif

#################compile tools
#ifeq ($(PLATFORM),arm)
#  TOOLS_DIR = /opt/freescale/usr/local/gcc-4.1.1-glibc-2.4-nptl-3/arm-none-linux-gnueabi/
#  CC = $(TOOLS_DIR)/bin/arm-none-linux-gnueabi-gcc-4.1.1
#	LD = $(TOOLS_DIR)/bin/arm-none-linux-gnueabi-gcc-4.1.1
#  AS = $(TOOLS_DIR)/arm_v6_vfp_le-as
#  AR = $(TOOLS_DIR)/arm_v6_vfp_le-ar
#
#  LIBS  = $(TOOLS_DIR)/arm-none-linux-gnueabi/lib/libc.a
#	LIBS += $(TOOLS_DIR)/arm-none-linux-gnueabi/lib/libm.a
#	LIBS += $(TOOLS_DIR)/arm-none-linux-gnueabi/lib/librt.a
##	LIBS += $(TOOLS_DIR)/arm-none-linux-gnueabi/lib/libpthread.a
#else
#  CC = gcc
#  AS = as
#  LD = ld
#  AR = ar
#
#  LIBS = libc.a libm.a librt.a libpthread.a
#endif

#################generate objectives
SRC0 =   $(wildcard $(SRCDIR0)/*.c)
SRC1 =   $(wildcard $(SRCDIR1)/*.c)
SRC2 =   $(wildcard $(SRCDIR2)/*.c)
OBJ =    $(SRC0:$(SRCDIR0)/%.c=$(OBJDIR)/%.o$(SUFFIX)) $(SRC1:$(SRCDIR1)/%.c=$(OBJDIR)/%.o$(SUFFIX))  \
         $(SRC2:$(SRCDIR2)/%.c=$(OBJDIR)/%.o$(SUFFIX))

#########################  targets start here  ##########################################
all: LIBRARY_COPY makedir bin copylib

#cp library from release directory to currrent directory
LIBRARY_COPY:
 ifeq ($(LIB_TYPE),STATIC)
	cp -f ../$(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).a  ./
 else
	cp -f ../$(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
 endif

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
#ifeq ($(BUILD), staticlib)
#	@echo 'creating static lib "$(BIN)"'
#	@$(AR) rc $(BIN) $(OBJ)
#endif
ifeq ($(BUILD), ARM11ELINUX)
	@echo 'creating share lib "$(BIN)"'
  ifneq ($(LIB_TYPE),STATIC)
	$(CC) -shared -o $(BIN) $(LFLAGS) $(OBJ) $(subst ../$(LIB_DIR)/,,$(CODEC_LIBRARY))
  else
	$(CC) -shared -o $(BIN) $(LFLAGS) $(OBJ) $(CODEC_LIBRARY)
  endif

endif
ifeq ($(BUILD), ARM9ELINUX)
	@echo 'creating share lib "$(BIN)"'
  ifneq ($(LIB_TYPE),STATIC)
	$(CC) -shared -o $(BIN) $(LFLAGS) $(OBJ) $(subst ../$(LIB_DIR)/,,$(CODEC_LIBRARY))
  else
	$(CC) -shared -o $(BIN) $(LFLAGS) $(OBJ) $(CODEC_LIBRARY)
  endif
endif
#ifeq ($(BUILD), ARM11ELINUXDLIB)
#	@echo 'creating share lib "$(BIN)"'
#	$(CC) -shared -o $(BIN) -lm -lc -lrt -mcpu=arm1136j-s $(OBJ) ./lib_H264_dec_arm11_elinux.so  $(LIBS)
#endif
#ifeq ($(BUILD), ARM9ELINUXDLIB)
#	@echo 'creating share lib "$(BIN)"'
#	$(CC) -Wl,-x -shared -o $(BIN) -lm -lc -lrt -mcpu=arm1136j-s $(OBJ) ./lib_H264_dec_arm9_elinux.so  $(LIBS)
#endif
#ifeq ($(BUILD), exe)
#	@echo 'creating exe binary "$(BIN)"'
#	@$(CC) -o $(BIN) $(OBJ) $(LIBS) $(LIB_DIR)/lib_H264_dec_arm11_elinux.a
#endif

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
	@#echo 'copy "$(BIN)" to share dir "$(CPYDIR)"'
	@#cp $(BIN) $(CPYDIR)
	@echo '... done'
	@echo