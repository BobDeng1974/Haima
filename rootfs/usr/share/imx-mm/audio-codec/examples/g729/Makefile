#*****************************************************************************
#
# G7.29 Test App Makefile
#
#*****************************************************************************
CODEC=g.729ab


include ../../build/Makefile_test.init

#Set the build specific variables

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
            #CFLAGS  += --inline --autoinline --forceinline
            ifeq ($(TGT_CPU),ARM9)
                CFLAGS   += -DTIME_PROFILE_RVDS_ARM9   # used for test ARM9 MIPS on RVDS
            else
                CFLAGS  += -DTIME_PROFILE_RVDS   # used for test ARM11 MIPS on RVDS
            endif
            CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
        else
            CFLAGS  += -DCYCLE_MEASUREMENT
        endif
    endif
endif


ifeq ($(TGT_CPU),ARM9)
    CFLAGS  +=  -DWall -D__sun
endif

ifeq ($(TGT_CPU),ARM11)
    CFLAGS  +=  -DWall -D__sun
endif

ifeq ($(TGT_OS),UNIX)
    #CFLAGS  += -DLOG_TIMING
endif

ifeq ($(TGT_OS),ELINUX)
    CFLAGS  += -O2
endif

ifeq ($(TGT_OS),LERVDS)
    ifeq ($(MAP),1)
        LFLAGS_DEC	+= --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map
        LFLAGS_ENC	+= --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map
    endif
    ifeq ($(ASM_FILE),1)
        CFLAGS += --asm
    endif
endif

C_DEC_OBJS = $(APP_DIR)/decoder/g729_dectest.o
C_ENC_OBJS = $(APP_DIR)/encoder/g729_enctest.o

ALL_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(C_ENC_OBJS)
DEC_OBJS = $(C_OBJS) $(C_DEC_OBJS)
ENC_OBJS = $(C_OBJS) $(C_ENC_OBJS)

all: LIBRARY_COPY TEST_$(TGT_CPU)_$(TGT_OS)
	@echo "--- Build-all done for library $(CODEC) ---"

LIBRARY_COPY:
ifndef DBG
	cp $(LIB_DIR)/lib_$(CODEC)_dec_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
	cp $(LIB_DIR)/lib_$(CODEC)_enc_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
endif

#Build the library

TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM9_LERVDS:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_BERVDS:$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS_DEC) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS_ENC) $(ENC_OBJS) $(LIBRARY_ENC)


TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_X86_UNIX : $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM_DEC)  $(PROGRAM_ENC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
