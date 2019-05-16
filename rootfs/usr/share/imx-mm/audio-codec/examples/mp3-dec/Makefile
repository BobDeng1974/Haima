#*****************************************************************************
#
# MP3 Decoder Test App Makefile
#
#*****************************************************************************
CODEC=mp3_dec
include ../../build/Makefile_test.init

CFLAGS += -DPUSH_MODE -DOUT_DUMP --debug
ifeq ($(TGT_CPU),ARM12)
    CFLAGS += -DARM12
endif

#Set the build specific variables

ifeq "$(TGT_OS)" "ELINUX"
    ifeq ($(TGT_CPU),ARM9)
	CFLAGS += -DMPEGLAYER1 -DMPEGLAYER2 -DUNIX_INTEGRATION -DDEBUG_API -DFILEIO -DUSE_MACROS -DLINUX #-DPRINT
	CFLAGS += -O2
        LFLAGS += -L$(LIB_DIR)  $(ENDIAN_$(TGT_ENDIAN))
    endif
    ifeq ($(TGT_CPU),ARM11)
	CFLAGS +=  -DMPEGLAYER1 -DMPEGLAYER2 -DFILEIO -DLINUX  -D$(TGT_ENDIAN)_ENDIAN #-DPRINT
    endif
    ifeq ($(TGT_CPU),ARM12)
	CFLAGS +=  -DMPEGLAYER1 -DMPEGLAYER2 -DFILEIO -DLINUX  -D$(TGT_ENDIAN)_ENDIAN #-DPRINT
    endif
    ifeq ($(TIME_PROFILE),1)
	    CPU_FREQ = 1000
    	CFLAGS +=  -DTIME_PROFILE -DCPUFREQ=$(CPU_FREQ)
		ifeq ($(TGT_CPU),ARM9)
	        CFLAGS   += -DARMPLAT=9 
	    endif
		ifeq ($(TGT_CPU),ARM11)
	        CFLAGS   += -DARMPLAT=11
	    endif
		ifeq ($(TGT_CPU),ARM12)
	        CFLAGS   += -DARMPLAT=12
	    endif
    endif
endif

ifeq ($(TGT_OS),LERVDS)
	CFLAGS  += -DMPEGLAYER1 -DMPEGLAYER2  -DFILEIO -DSINGLE_VECTOR --inline --autoinline --forceinline -DOUT_DUMP #-DPRINT -DARM_ADS
	ifeq ($(TIME_PROFILE),1)
			CFLAGS += -DTIME_PROFILE_RVDS
			CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
			COMMON_DIR_C = ../../common/c_src
			C_OBJS += $(COMMON_DIR_C)/profile_utils.o
			INCLUDES += -I./c_src
	endif

	ifeq ($(MX31ADS_STANDALONE),1)
	    AFLAGS += --PD "MX31ADS SETL {TRUE}"
	    LFLAGS += --scatter ../../common/asm_arm/mx31/scat_a.txt --entry 0x80080000
	    ifeq ($(LEVEL2_PAGE),1)
	    	AFLAGS += --PD "L2_TTB SETL {TRUE}"
	    endif
	endif
	ifeq ($(MX353DS_STANDALONE),1)
	    AFLAGS += --PD "MX353DS SETL {TRUE}"
	    LFLAGS += --scatter ../../common/asm_arm/mx35/scat_a.txt --entry 0x80080000
	    ifeq ($(LEVEL2_PAGE),1)
	    	AFLAGS += --PD "L2_TTB SETL {TRUE}"
	    endif
	endif

		CFLAGS  += -DLITTLE_ENDIAN
endif

ifeq ($(TGT_OS),UNIX)
	CFLAGS += -DMPEGLAYER1 -DMPEGLAYER2 -DSINGLE_VECTOR -DFILEIO  -DPRINT -D$(TGT_ENDIAN)_ENDIAN
endif


C_OBJS = $(APP_DIR)/mp3_dec_test.o


ifeq ($(TGT_OS),LERVDS)
 ifeq ($(TIME_PROFILE),1)
  C_OBJS += ../../common/c_src/profile_utils.o
 endif
 ifeq ($(MX31ADS_STANDALONE),1)
	C_OBJS += ../../common/c_src/retarget.o
  ASM_OBJS += ../../common/asm_arm/mx31/initmmu.o
  ASM_OBJS += ../../common/asm_arm/mx31/ttb.o
 endif

 ifeq ($(MX353DS_STANDALONE),1)
	C_OBJS += ../../common/c_src/retarget.o
  ASM_OBJS += ../../common/asm_arm/mx35/initmmu.o
  ASM_OBJS += ../../common/asm_arm/mx35/ttb.o
 endif
endif

ifeq ($(TGT_CPU),ARM12)
ifneq "$(TGT_OS)" "ELINUX"
  C_OBJS += ../../common/c_src/retarget_Cortex.o
  ASM_OBJS += ../../common/asm_arm/Init_Cortex.o \
              ../../common/asm_arm/InitCache_Cortex.o \
              ../../common/asm_arm/Vectors.o
endif
endif

ALL_OBJS = $(C_OBJS) $(ASM_OBJS)

all: LIBRARY_COPY TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"

LIBRARY_COPY:
ifndef DBG
 ifeq ($(LIB_TYPE),STATIC)
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).a  ./
 else
	cp $(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
 endif
endif

#Build the library
TEST_ARM12_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))

TEST_ARM12_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))


TEST_ARM9_ELINUX	: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
ifneq ($(BUILD),ARM9GSELINUX)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
else
ifeq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY)) $(GSO_LIBS)
endif
ifeq ($(LIB_TYPE),DYNAMIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst ../../src/$(CODEC)/,,$(LIBRARY))
endif
endif


TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM)  $(PROGRAM).map main.in
