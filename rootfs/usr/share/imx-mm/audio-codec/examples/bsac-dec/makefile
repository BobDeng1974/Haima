
#*****************************************************************************
#
#  bsac_dec Codec Makefile
#
#*****************************************************************************
CODEC=bsac_dec


include ../../build/Makefile_test.init

CFLAGS += -DCT_CONF #-DINTERLEAVE_OUTPUT

ifdef RAW_INPUT
CFLAGS += -DRAW_INPUT -DxNO_STD_LIB
endif

CFLAGS  += -o2 -DBSAC_DEC #-DINTERLEAVE_OUTPUT
ifeq ($(BSAC_DEBUG),1)
        CFLAGS  += -DBSAC_DEBUG_DUMP
endif

ifneq ($(TEST_BLOCK_OUTPUT), 1)
       CFLAGS  += -DTEST_INTERLEAVE_OUTPUT
endif
ifeq ($(TGT_OS),ELINUX)
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

  ifeq ($(TGT_CPU),ARM11)
	CFLAGS   += -DMPEG4 -DARM_OPT_MACROS -DLC
 endif

 ifeq ($(TGT_CPU),ARM9)

     CFLAGS += -O2 -DLC  -DFIXED_POINT_CODE -DMPEG4 -DARM_OPT_MACROS -DLITTLE_ENDIAN -D_fix_h
 endif
endif
ifeq ($(TGT_OS),SYMBIAN)
	ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE
	endif
	ifeq ($(MEMORY_MEASUREMENT),1)
		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
	endif

  ifeq ($(TGT_CPU),ARM11)
	CFLAGS   += -DMPEG4 -DARM_OPT_MACROS -DLC  -DSINGLE_VECTOR
 endif

 endif
ifeq ($(TGT_OS),UNIX)

	CFLAGS  += -DUNIX

endif

ifeq ($(TGT_OS),VRTX)
  CFLAGS  += --pointer_alignment=1 --min_array_alignment=4 --debug -O0 --apcs=interwork --memaccess=-UL41\
             -DMPEG4 -DARM_OPT_MACROS -DLC -DARM_ADS -DARM_DCT_ASM -DARM_FFT_ASM -DCO_PROCESSOR_C15 \
            -DFIXED_POINT_CODE -DNO_STD_LIB -DSINGLE_VECTOR -DOS_VRTX -DMHZ_MEASUREMENT

 AFLAGS += --debug --keep --xref --apcs=inter --predefine="EMBEDDED_ARM SETA 1" \
          --predefine="CO_PROCESSOR_C15 SETL {TRUE}"

  LFLAGS += --be32 --ro_base=0x90000000

endif

ifeq ($(TGT_OS),LERVDS)

  CFLAGS  += -DARM_ADS  -DSINGLE_VECTOR -DFLEXIBLE_VECTOR_PATH --inline --autoinline --forceinline  #-DDISCARD_OUTPUT #--enum_is_int
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

	LFLAGS +=  --info sizes,totals --info libraries --info unused --symbols --map --list ./bsacd_arm.map
endif

ifeq ($(TGT_OS),BERVDS)

CFLAGS  +=  --enum_is_int --pointer_alignment=1 --min_array_alignment=4 --debug -O0 -Otime --apcs=interwork --memaccess=-UL41\
          -DMPEG4 -DARM_OPT_MACROS -DLC -DARM_ADS -DARM_DCT_ASM -DARM_FFT_ASM -DCO_PROCESSOR_C15 \
          -DFIXED_POINT_CODE -DNO_STD_LIB -DSINGLE_VECTOR -DMHZ_MEASUREMENT -DBE_RVDS -DFLEXIBLE_VECTOR_PATH


AFLAGS +=  --bigend --debug --keep --xref --apcs=inter --predefine="EMBEDDED_ARM SETA 1" \
          --predefine="CO_PROCESSOR_C15 SETL {TRUE}"

LFLAGS += --be32 --ro_base=0x90000000


endif



 C_OBJS = $(APP_DIR)/bsac_main.o
 C_OBJS +=$(APP_DIR)/aac_aux.o
 C_OBJS += $(APP_DIR)/portio.o

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



ifeq ($(TGT_OS),VRTX)
  ASM_OBJS += ../../common/asm_arm/rt_memory.o
  ASM_OBJS += ../../common/asm_arm/rt_mem_h.o

endif
ifeq ($(TGT_OS),BERVDS)
  ASM_OBJS += ../../common/asm_arm/rt_memory.o
  ASM_OBJS += ../../common/asm_arm/rt_mem_h.o

endif



ifeq ($(TGT_OS),UNIX)
	ALL_OBJS = $(C_OBJS)
else
	ALL_OBJS = $(C_OBJS) $(ASM_OBJS)
endif





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
TEST_ARM11_SYMBIAN: $(ALL_OBJS) $(LIBRARY)
		$(LN) $(INFLAGS) $(ALL_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM) --symbols --list $(PROGRAM).map  $(LIBRARY) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM)  $(PROGRAM)

TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY)
#		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -Wl,-x -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))


TEST_ARM9_ELINUX	: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
ifneq ($(BUILD),ARM9GSELINUX)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
else
ifeq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY) $(GSO_LIBS)
endif
ifeq ($(LIB_TYPE),DYNAMIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
endif
endif

TEST_X86_UNIX	: $(ALL_OBJS) $(LIBRARY)
		$(LN) -o  $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM) $(PROGRAM).map main.in
