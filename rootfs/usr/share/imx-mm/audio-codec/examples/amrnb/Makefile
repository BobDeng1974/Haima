#*****************************************************************************
#
# NBAMR Codec Makefile
#
#*****************************************************************************
CODEC=nb_amr


include ../../build/Makefile_test.init


#Set the build specific variables


ifeq ($(TOOL_CHAIN),MVISTA)
  ifeq ($(TGT_OS),ELINUX)
	CFLAGS	+= -msoft-float -mfpu=vfp
  endif
endif

ifeq ($(TOOL_CHAIN),RVDS)
  ifeq ($(TGT_OS),VRTX)
	CFLAGS	+= --debug -O0 --apcs=interwork --memaccess=-UL41 -DARM_OPTS -DWMOPS=0 \
		-DOS_VRTX -DTARGET_ARM11 -DCYCLE_MEASUREMENT
	AFLAGS 	+= --debug --keep --apcs=inter --memaccess=-UL41
      LFLAGS	+= --be32 --ro_base=0x90000000
  endif
endif

ifeq ($(TGT_OS),LERVDS)
	CFLAGS += -D__sun__  --inline --autoinline --forceinline
        #-O3 is not support in test
	CFLAGS += -O0
	ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE_RVDS
		CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
	endif
        ifeq ($(MAP),1)
           LFLAGS_DEC	+= --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map
           LFLAGS_ENC	+= --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map
        endif
        ifeq ($(ASM_FILE),1)
            CFLAGS += --asm
        endif

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

endif

ifeq ($(TGT_OS),SYMBIAN)

ifeq ($(TIME_PROFILE),1)

CFLAGS += -DTIME_PROFILE

endif
       	ifeq ($(MEMORY_MEASUREMENT),1)
        		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
       	endif
endif





CFLAGS += -DARM_OPTS -DWMOPS=0 -D__sun_ -DARMGCC_OPTS

#CFLAGS +=-O2 -DTARGET_ARM9 -D__sun__ -DARMGCC_OPTS -DWMOPS=0 -fomit-frame-pointer -DARM_OPTS
# Define this to output cycles information(on armulator)
#CFLAGS += -DCYCLE_MEASUREMENT
# Define this to output cycles information(on board)
#CFLAGS += -DSYSTEM_TIME
#Define this for alignment check
#CFLAGS += -DALIGNMENT_CHECK

ifeq ($(TGT_ENDIAN),BIG)
  CFLAGS += -DNBAMR_BIG_ENDIAN -DNBAMR_ENDIAN_BIG
  AFLAGS += --predefine="NBAMR_ENDIAN_BIG SETL {TRUE}"
endif

ifeq ($(TGT_OS),BERVDS)
 AFLAGS += --debug --keep  --predefine="NBAMR_ENDIAN_BIG SETL {TRUE}" --apcs=inter --memaccess=-UL41

 CFLAGS  += --debug -O0 -Otime -DTARGET_ARM11 -DCYCLE_MEASUREMENT \
		--apcs=interwork -DARM_OPTS -DWMOPS=0 -DNBAMR_ENDIAN_BIG  --memaccess=-UL41

 LFLAGS += --be32 --ro_base=0x90000000
endif


ifeq ($(TGT_CPU),ARM11)
  CFLAGS += -DTARGET_ARM11

  ifeq ($(TGT_OS),ELINUX)
    CFLAGS += -D__sun__ -DARMGCC_OPTS
  endif

  ifeq ($(TGT_OS),VRTX)
    CFLAGS += -DOS_VRTX
  endif
endif
ifeq ($(TGT_CPU),ARM9)
ifeq ($(TGT_OS),ELINUX)
    CFLAGS += -funroll-all-loops -fomit-frame-pointer
endif
endif

C_OBJS = $(APP_DIR)/common/log_api.o

ifeq ($(TGT_OS),ELINUX)
 ifeq ($(TGT_CPU),ARM11)
     C_OBJS += $(APP_DIR)/common/table_size.o
 endif
endif
ifeq ($(TGT_OS),SYMBIAN)
 ifeq ($(TGT_CPU),ARM11)
     C_OBJS += $(APP_DIR)/common/table_size.o
 endif
endif
ifeq ($(TGT_OS),VRTX)
  ASM_OBJS =  ../../common/asm_arm/rt_memory.o \
		  ../../common/asm_arm/rt_mem_h.o

endif

ifeq ($(TGT_OS),BERVDS)
  ASM_OBJS =  ../../common/asm_arm/rt_memory.o \
		  ../../common/asm_arm/rt_mem_h.o

endif

C_DEC_OBJS = $(APP_DIR)/decoder/nbamr_dectest.o
C_ENC_OBJS = $(APP_DIR)/encoder/nbamr_enctest.o



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

TEST_ARM11_LERVDS:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_LERVDS:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_BERVDS:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
#	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
#	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
  ifneq ($(BUILD),ARM9GSELINUX)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
  else
     ifeq ($(LIB_TYPE),DYNAMIC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
     endif
     ifeq ($(LIB_TYPE),STATIC)
        $(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(GSO_LIBS)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC)) $(GSO_LIBS)
     endif
  endif


TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM_DEC)  $(PROGRAM_ENC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
