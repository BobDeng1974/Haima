#*****************************************************************************
#
# G7.26 Test App Makefile
#
#*****************************************************************************
CODEC=g.726


include ../../build/Makefile_test.init

#Set the build specific variables
CFLAGS += -DG726_DEBUG_PRINT
#CFLAGS += -DTIME_PROFILE

ifeq ($(BIT_MATCH),1)
    CFLAGS += -DBIT_MATCH_WITH_REF
endif

ifeq ($(TIME_PROFILE), 1)
    ifeq ($(TGT_OS),ELINUX)
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
    else
        ifeq ($(TGT_OS),LERVDS)
            CFLAGS  += -DTIME_PROFILE_RVDS             # used for both ARM9 and ARM11
            ifeq ($(TGT_CPU),ARM9)
                CFLAGS   += -DTIME_PROFILE_RVDS_ARM9   # used for test ARM9 MIPS on RVDS
            else
                #CFLAGS  += -DTIME_PROFILE_RVDS_ARM11   # used for test ARM11 MIPS on RVDS
                CFLAGS  += -DTIME_PROFILE_RVDS_ARM9   # used for test ARM11 MIPS on RVDS
            endif
            #CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
        else
            CFLAGS  += -DCYCLE_MEASUREMENT
        endif
    endif
endif


ifeq ($(TGT_OS),SYMBIAN)
	ifeq ($(TIME_PROFILE), 1)
		CFLAGS  += -DTIME_PROFILE
	endif
	ifeq ($(MEMORY_MEASUREMENT),1)
    		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
   	endif
endif

ifeq ($(TGT_OS),BERVDS)
  AFLAGS += --debug --keep --cpu=ARM1136J-S --apcs=inter
  CFLAGS += --debug -O0 -Otime --list --pointer_alignment=1 --min_array_alignment=4\
		-O0 -Otime --apcs=interwork --memaccess=-UL41 -DTARGET_G726_ARM11 -DENDIAN_BIG
  LFLAGS += --be32 --ro_base=0x90000000
endif

ifeq ($(TGT_OS),VRTX)
     CFLAGS +=  --debug -O0 --pointer_alignment=1 --min_array_alignment=4 -DCYCLE_MEASUREMENT \
		  --apcs=interwork --memaccess=-UL41 -DTARGET_G726_ARM11 -DG726_BIG_ENDIAN -DOS_VRTX
     AFLAGS += --debug --keep  --apcs=inter
     LFLAGS	+= --be32 --ro-base=0x90000000
endif

C_OBJS = $(APP_DIR)/common/strfunc.o		\
		$(APP_DIR)/common/log_api.o

ifeq ($(TGT_OS),VRTX)
    C_OBJS += $(APP_DIR)/common/staticmemory.o
    ASM_OBJS +=../../common/asm_arm/rt_mem_h.o
endif
ifeq ($(TGT_OS),BERVDS)
    C_OBJS += $(APP_DIR)/common/staticmemory.o
    ASM_OBJS +=../../common/asm_arm/rt_mem_h.o
endif





C_DEC_OBJS = $(APP_DIR)/decoder/g726_dectest.o
C_ENC_OBJS = $(APP_DIR)/encoder/g726_enctest.o



ALL_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(C_ENC_OBJS)
DEC_OBJS = $(C_OBJS) $(C_DEC_OBJS)
ENC_OBJS = $(C_OBJS) $(C_ENC_OBJS)
ifeq ($(TGT_OS),VRTX)
ALL_OBJS += $(ASM_OBJS)
DEC_OBJS += $(ASM_OBJS)
ENC_OBJS += $(ASM_OBJS)
endif



all: LIBRARY_COPY TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"

LIBRARY_COPY:
ifndef DBG
	cp $(LIB_DIR)/lib_$(CODEC)_dec_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
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
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)
	#$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map $(LIBRARY_DEC) $(SYMLIB)
	#$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map $(LIBRARY_ENC) $(SYMLIB)

TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_LERVDS: $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)
	#$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map $(LIBRARY_DEC) $(SYMLIB)
	#$(LN) -o $(PROGRAM_ENC) $(LFLAGS)  $(ENC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map $(LIBRARY_ENC) $(SYMLIB)

TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
#	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
#	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))


TEST_X86_UNIX : $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM_DEC)  $(PROGRAM_ENC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
