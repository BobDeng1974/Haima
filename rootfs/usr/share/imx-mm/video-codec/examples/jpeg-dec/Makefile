#*****************************************************************************
#
# jpeg_dec Test App Makefile
#
#*****************************************************************************
CODEC=jpeg_dec


# NOTICE for building jpeg_dec, need vpu related lib and hdr
# imx-lib contains the VPU libraries and binaries
# copy to the hdr directory, the vpu_lib.h and vpu_io.h  from imx-lib package or from /usr/lib  from target
# copy to the vpu directory the libvpu.so and libvpu.a


include ../../build/Makefile_test.init

## iMX51,iMX53,...   : USE_VPU_LIB=1
## iMX233,...: USE_VPU_LIB=1 (because no suffix '_novpu' for arm9)
USE_VPU_LIB=1
## iMX508,...: USE_VPU_LIB=0
#USE_VPU_LIB=0

INCLUDES += -I./hdr

ifeq ($(USE_VPU_LIB),1)
 #INCLUDES += -I../../src/jpeg_dec/hdr/vpu/linux
 CFLAGS+=-DVPU_SUPPORT
 ifneq ($(TGT_CPU), ARM9)
    #LIBRARY +=$(TOOLS_DIR)/arm-none-linux-gnueabi/lib/libpthread.a ./libvpu.a
    LFLAGS += -L./vpu -lvpu
    #LFLAGS+=-L./vpu
    #LIBRARY +=./vpu/libvpu.a
 endif
else
 CFLAGS+=-DVPU_SUPPORT
 PROGRAM = test_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN)_novpu
  ifeq ($(LIB_TYPE),DYNAMIC)
    LIBRARY = $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN)_novpu.so $(TC_LIBRARY)
  else
    LIBRARY = $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN)_novpu.a $(TC_LIBRARY)
  endif      
endif

ifeq  ($(TGT_OS),ELINUX)
  ifeq ($(TIME_PROFILE),1)
	CFLAGS +=  -DTIME_PROFILE -DCHECK_CYCLES
  endif
endif

ifeq  ($(TGT_OS),SYMBIAN)
  ifeq ($(TIME_PROFILE),1)
	CFLAGS +=  -DTIME_PROFILE
  endif
endif

ifeq  ($(TGT_OS),SYMBIAN)
  ifeq ($(TIME_PROFILE),1)
	CFLAGS +=  -DTIME_PROFILE
  endif
         	ifeq ($(MEMORY_MEASUREMENT),1)
          		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
       	endif
endif
ifeq ($(TGT_OS),LERVDS)
	CFLAGS   += --no_inline --no_autoinline
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE_RVDS -DCHECK_CYCLES
		CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
	endif
endif

ifeq ($(TGT_OS),BERVDS)
 AFLAGS +=  --debug --keep
 CFLAGS += --dwarf2 --debug  -O0 -Otime --apcs=inter --memaccess=-UL41 \
	     -DARM11_CCM -DJPEG_DEC_IDCT_ASM -DBE_RVDS

 LFLAGS += --be32
 	ifeq ($(TIME_PROFILE),1)
 		CFLAGS +=  -DTIME_PROFILE_RVDS -DCHECK_CYCLES
 		COMMON_DIR_C = ../../common/c_src
 		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
 		INCLUDES += -I./c_src
 	endif

endif

ifeq ($(TGT_OS), VRTX)
# INCLUDES += -I../../src/jpeg_dec/hdr

 AFLAGS  +=   --debug --keep
 CFLAGS  += --dwarf2 --debug  -O0 --apcs=inter --memaccess=-UL41 \
	     -DARM11_CCM -DJPEG_DEC_IDCT_ASM -DOS_VRTX #-D_SIM

 LFLAGS += --be32 --ro_base=0x96000000
endif

#C_OBJS += $(APP_DIR)/wrbmp.o
#C_OBJS += $(APP_DIR)/log_api.o
C_OBJS += $(APP_DIR)/jpeg_dec_app.o


ifeq ($(TGT_OS), VRTX)
ASM_OBJS  = ../../common/asm_arm/rt_memory.o
ASM_OBJS += ../../common/asm_arm/rt_mem_h.o
ASM_OBJS += ../../common/asm_arm/initcache.o
endif
ifeq ($(TGT_OS), BERVDS)
ASM_OBJS  = ../../common/asm_arm/rt_memory.o
ASM_OBJS += ../../common/asm_arm/rt_mem_h.o
ASM_OBJS += ../../common/asm_arm/initcache.o
endif

ifeq ($(FLOAT),HW)
  INCDIR+= -I$(SYS_INCLUDE)
  FLAGS += -ftree-vectorize -ffast-math -fsingle-precision-constant -mfloat-abi=hard -DFHW
  LIBS += --sysroot=/usr/armv7-poky-gnueabi-4.7.2-tool-chain/imx6qsabresd
  #PROGRAM := $(PROGRAM)_fhw
endif

ALL_OBJS = $(C_OBJS)


all: LIBRARY_COPY TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"


LIBRARY_COPY:
ifndef DBG
ifeq ($(USE_VPU_LIB),1)
 ifeq ($(LIB_TYPE),STATIC)
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).a  ./
 else
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
 endif
else
 ifeq ($(LIB_TYPE),STATIC)
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN)_novpu.a  ./
 else
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN)_novpu.so ./
 endif
endif
endif

#Build the library
TEST_ARM11_SYMBIAN: $(ALL_OBJS) $(LIBRARY)
		$(LN) $(INFLAGS) $(ALL_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM) --symbols --list $(PROGRAM).map  $(LIBRARY) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM)  $(PROGRAM)

TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)



TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY)
#		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


TEST_ARM11_ELINUX: 	$(ALL_OBJS)  $(LIBRARY)
ifneq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
else
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
endif


TEST_ARM9_ELINUX	: $(ALL_OBJS) $(LIBRARY)
ifneq ($(BUILD),ARM9GSELINUX)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
else
ifeq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) $(GSO_LIBS)
endif
ifeq ($(LIB_TYPE),DYNAMIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
endif
endif


TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM)  $(PROGRAM).map main.in


