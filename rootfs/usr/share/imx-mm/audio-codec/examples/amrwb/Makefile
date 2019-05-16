
#*****************************************************************************
#
# WBAMR Codec Makefile
#
#*****************************************************************************
CODEC=wb_amr


include ../../build/Makefile_test.init

ifeq ($(TGT_CPU),ARM12)
    CFLAGS += -DTGT_CPU_ARM12
endif

#Set the build specific variables

ifeq ($(TGT_OS),ELINUX)
	ifeq ($(TGT_CPU),ARM11)
    		CYCLE =
    		CFLAGS    +=-DTGT_CPU_ARM11
 	endif

 	ifeq ($(TGT_CPU),ARM12)
    		CYCLE =
    		CFLAGS    +=-DTGT_CPU_ARM11
 	endif

    	ifeq ($(TGT_CPU),ARM9)
     		CFLAGS    += -DTGT_CPU_ARM9
    	endif

	ifeq ($(TIME_PROFILE),1)
		CPU_FREQ = 1000
		CFLAGS += -DTIME_PROFILE -DCPUFREQ=$(CPU_FREQ)
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
    CFLAGS    += -DFILEIO -DLITTLE_ENDIAN  -D__MSDOS__ -DWBAMR_ARM1136
endif

ifeq ($(TGT_OS),SYMBIAN)
	ifeq ($(TGT_CPU),ARM11)
    		CYCLE =
    		CFLAGS    +=-DTGT_CPU_ARM11
 	endif
 	ifeq ($(TGT_CPU),ARM12)
    		CYCLE =
    		CFLAGS    +=-DTGT_CPU_ARM11
 	endif

    	ifeq ($(TGT_CPU),ARM9)
     		CFLAGS    += -DTGT_CPU_ARM9
    	endif

	ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE=1
	endif
	ifeq ($(MEMORY_MEASUREMENT),1)
	 	CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
       	endif
    CFLAGS    += -DFILEIO -DLITTLE_ENDIAN  -D__MSDOS__ -DWBAMR_ARM1136 -O0
endif

ifeq ($(TGT_OS),UNIX)
	CYCLE =
	CFLAGS   += -D__MSDOS__
endif

ifeq ($(TGT_OS),LERVDS)

	CFLAGS  += -D__MSDOS__ -DWBAMR_ARM1136 -O0 --inline --autoinline --forceinline

        ifeq ($(MAP),1)
            LFLAGS_DEC   += --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map
            LFLAGS_ENC   += --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map
        endif
        ifeq ($(ASM_FILE),1)
            CFLAGS += --asm
        endif

	ifeq ($(TIME_PROFILE),1)
            ifeq ($(TGT_CPU),ARM9)
		CFLAGS += -DTIME_PROFILE_RVDS_ARM9
            else
		CFLAGS += -DTIME_PROFILE_RVDS
            endif
		CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
	endif
endif

ifeq ($(TGT_ENDIAN),BIG)
  CFLAGS  +=  -DWBAMR_BIG_ENDIAN
endif

ifeq ($(TOOL_CHAIN),RVDS)
  ifeq ($(TGT_OS),VRTX)

   AFLAGS +=  --keep --fpu=None \
		  --predefine="WBAMR_ARM1136 SETL {TRUE}" --predefine="ENDIAN_BIG SETL {TRUE}"

   CFLAGS  += -c -g -O0 -D__MSDOS__ -DWBAMR_ARM1136 -DENDIAN_BIG -DSIM_RVDS22 -DCYCLE_MEASUREMENT1 -DOS_VRTX

   LFLAGS += --be32 --ro_base=0x92000000

  endif
endif

ifeq ($(TGT_OS),VRTX)
 AFLAGS += --keep --fpu=None --predefine="WBAMR_ARM1136 SETL {TRUE}" --predefine="ENDIAN_BIG SETL {TRUE}"

 CFLAGS += -c -g -S -O0 -Otime -D__MSDOS__ -DWBAMR_ARM1136 -DENDIAN_BIG -DSIM_RVDS22 -DCYCLE_MEASUREMENT1 -DOS_VRTX

 LFLAGS += --be32 --ro_base=0x92000000
endif

ifeq ($(TGT_OS),BERVDS)

AFLAGS +=  --keep --fpu=None --predefine="WBAMR_ARM1136 SETL {TRUE}" --predefine="ENDIAN_BIG SETL {TRUE}"

CFLAGS += -c -g -O0 -Otime -D__MSDOS__ -DWBAMR_ARM1136 -DENDIAN_BIG -DSIM_RVDS22 -DCYCLE_MEASUREMENT1 -DBE_RVDS

LFLAGS += --be32 --ro_base=0x92000000
endif





  C_OBJS += $(APP_DIR)/common/dummycall.o

  C_OBJS +=       $(APP_DIR)/common/log_api.o

ifeq ($(TGT_OS),VRTX)
 C_OBJS +=../../common/c_src/profile_utils.o
endif

ifeq ($(TGT_OS),BERVDS)
 C_OBJS +=../../common/c_src/profile_utils.o
endif


C_DEC_OBJS = $(APP_DIR)/decoder/wbamr_dectest.o
C_ENC_OBJS = $(APP_DIR)/encoder/wbamr_enctest.o

ifeq ($(TGT_OS),VRTX)
  ASM_OBJS =  ../../common/asm_arm/rt_memory.o \
		  ../../common/asm_arm/rt_mem_h.o

endif

ifeq ($(TGT_CPU),ARM12)
ifneq "$(TGT_OS)" "ELINUX"
  C_OBJS += ../../common/c_src/retarget_Cortex.o
  ASM_OBJS += ../../common/asm_arm/Init_Cortex.o \
              ../../common/asm_arm/InitCache_Cortex.o \
              ../../common/asm_arm/Vectors.o
endif
endif


ALL_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(C_ENC_OBJS) $(ASM_OBJS)
DEC_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(ASM_OBJS)
ENC_OBJS = $(C_OBJS) $(C_ENC_OBJS) $(ASM_OBJS)



all: LIBRARY_COPY TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"

LIBRARY_COPY:
ifndef DBG
	cp $(LIB_DIR)/lib_$(CODEC)_dec_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so  ./
	cp $(LIB_DIR)/lib_$(CODEC)_enc_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
endif

#Build the library
TEST_ARM11_SYMBIAN: $(ALL_OBJS) $(LIBRARY_ENC) $(LIBRARY_DEC)
		$(LN) $(INFLAGS) $(DEC_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_DEC) --symbols --list $(PROGRAM_DEC).map  $(LIBRARY_DEC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_DEC)  $(PROGRAM_DEC)
		$(LN) $(INFLAGS) $(ENC_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_ENC) --symbols --list $(PROGRAM_ENC).map  $(LIBRARY_ENC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_ENC)  $(PROGRAM_ENC)

TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM12_LERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM12_BERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_BERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
#	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
#	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM12_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))


TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))


TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))


TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM_DEC) $(PROGRAM_ENC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
