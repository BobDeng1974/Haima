#*****************************************************************************
#
# png_dec Test App Makefile
#
#*****************************************************************************
CODEC=png_dec



include ../../build/Makefile_test.init


ifeq ($(TGT_OS),ELINUX)
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE
    else
        CFLAGS += -DSAVE_OUTPUT
    endif
endif


ifeq ($(TGT_OS),SYMBIAN)
       	ifeq ($(MEMORY_MEASUREMENT),1)
       		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
       		TIME_PROFILE = 1
       	endif
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE 
    else
        CFLAGS += -DSAVE_OUTPUT
    endif
endif

ifeq ($(TGT_OS),LERVDS)
	CFLAGS   += --inline --autoinline --forceinline
	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE_RVDS
        CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE	
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
    else
        CFLAGS += -DSAVE_OUTPUT
	endif
endif

ifeq ($(TGT_OS),BERVDS)
	
AFLAGS +=  --debug --keep --apcs=inter --memaccess=-UL41
CFLAGS += --debug -O0 -Otime -DENDIAN_BIG --list --apcs=inter --memaccess=-UL41
LFLAGS += --be32
  ifeq ($(TIME_PROFILE),1)
	CFLAGS +=  -DTIME_PROFILE_RVDS
	COMMON_DIR_C = ../../common/c_src
	C_OBJS += $(COMMON_DIR_C)/profile_utils.o
	INCLUDES += -I./c_src
  else
    CFLAGS += -DSAVE_OUTPUT
  endif
endif


ifeq ($(TGT_OS),VRTX)

CFLAGS  += --debug  -O0 -DENDIAN_BIG --apcs=inter --memaccess=-UL41 -DOS_VRTX -DSAVE_OUTPUT
AFLAGS += --debug --keep  --apcs=inter --memaccess=-UL41
LFLAGS += --be32 --ro_base=0x90000000

endif

C_OBJS += $(APP_DIR)/png_test_wrapper.o
C_OBJS += $(APP_DIR)/log_api.o

ifeq ($(TGT_OS),VRTX)

C_OBJS += ../../common/c_src/profile_utils.o

ASM_OBJS =  ../../common/asm_arm/rt_memory.o
ASM_OBJS += ../../common/asm_arm/initcache.o

endif

ifeq ($(TIME_PROFILE),0)
CFLAGS += -DSAVE_OUTPUT
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
TEST_ARM11_SYMBIAN: $(ALL_OBJS)  $(LIBRARY) 
		$(LN) $(INFLAGS) $(ALL_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM) --symbols --list $(PROGRAM).map $(SYMLIB)  $(LIBRARY) 
		$(ELFTRAN)   $(PROGRAM)  $(PROGRAM)


TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY) 
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 

TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY) 
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 
		

TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY) 
#		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 
		

TEST_ARM11_ELINUX: $(ALL_OBJS) $(LIBRARY) 
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 
		
TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY) 
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) 

TEST_ARM9_BERVDS:
	@echo "--- Build not supported for library $(CODEC) ---"

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





