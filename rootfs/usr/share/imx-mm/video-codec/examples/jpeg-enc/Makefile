#*****************************************************************************
#
# jpeg_enc Test App Makefile
#
#*****************************************************************************
CODEC=jpeg_enc



include ../../build/Makefile_test.init


ifndef API

    API=FRAME
endif
 
FRAME_EXTN=frame
ROW_EXTN=mcu

ifeq ($(TGT_OS),ELINUX)
    ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE
    endif
 endif


ifeq ($(TGT_OS),SYMBIAN)
    ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE
    endif
       	ifeq ($(MEMORY_MEASUREMENT),1)
            		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
       	endif
 endif
 
ifeq ($(TGT_OS),UNIX)
   CFLAGS   += --inline 
endif

ifeq ($(TGT_OS),LERVDS)
	CFLAGS   += --inline --autoinline --forceinline -DRVDS_TEST
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE_RVDS
		CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
	endif
endif

ifeq ($(TGT_OS),BERVDS)
	
  CFLAGS   += -DRVDS_TEST

  AFLAGS += --debug --keep --predefine="FAST_DCT_ASM SETL {TRUE}" --predefine="TARGET_ARM_1136J_S SETL {TRUE}"\
	      --predefine="ASM SETL {TRUE}" --predefine="ENABLE_UNALIGNED_ACCESS SETL {FALSE}" --predefine="ENDIAN_BIG SETL {TRUE}"

 CFLAGS  +=  --debug --cpu=ARM1136J-S -O3 -Otime -DNOERREXIT -DFAST_DCT_ASM -DTARGET_ARM_1136J_S -DARM_ADS -DASM\
		 -DCCM -DEXIF -DENDIAN_BIG  --list -DRVDS_TEST

LFLAGS += --be32
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE_RVDS
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
	endif
endif



#Set the build specific variables
  
ifeq ($(TGT_OS), VRTX)
  AFLAGS +=  --debug --keep \
	     --predefine="FAST_DCT_ASM SETL {TRUE}"\
	     --predefine="TARGET_ARM_1136J_S SETL {TRUE}"\
	     --predefine="ASM SETL {TRUE}" --predefine="ENABLE_UNALIGNED_ACCESS SETL {FALSE}"\
	     --predefine="ENDIAN_BIG SETL {TRUE}"


  CFLAGS  += --debug -DNOERREXIT\
	     -DFAST_DCT_ASM -DTARGET_ARM_1136J_S -DARM_ADS -DASM\
	     -DCCM -DEXIF -DENDIAN_BIG -DOS_VRTX 

  LFLAGS += --be32 --map --ro_base=0x90000000
endif


C_OBJS += $(APP_DIR)/application.o
C_OBJS += $(APP_DIR)/log_api.o

ifeq ($(TGT_OS), VRTX)
 ASM_OBJS = ../../common/asm_arm/rt_memory.o
 ASM_OBJS += ../../common/asm_arm/rt_mem_h.o
 ASM_OBJS += ../../common/asm_arm/initcache.o
endif
ifeq ($(TGT_OS), BERVDS)
 ASM_OBJS = ../../common/asm_arm/rt_memory.o
 ASM_OBJS += ../../common/asm_arm/rt_mem_h.o
 ASM_OBJS += ../../common/asm_arm/initcache.o
 ASM_OBJS += ../../common/c_src/profile_utils.o

endif

ifeq ($(FLOAT),HW)
  INCDIR+= -I$(SYS_INCLUDE)
  FLAGS += -ftree-vectorize -ffast-math -fsingle-precision-constant -mfloat-abi=hard -DFHW
  LIBS += --sysroot=/usr/armv7-poky-gnueabi-4.7.2-tool-chain/imx6qsabresd
  PROGRAM := $(PROGRAM)_fhw
endif
	
ALL_OBJS = $(C_OBJS) $(ASM_OBJS) 


all: 	TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"

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
		

TEST_ARM11_ELINUX: $(ALL_OBJS) $(LIBRARY) 
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 
		

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




