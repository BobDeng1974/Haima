#*****************************************************************************
#
# G.723.1 Test App Makefile
#
#*****************************************************************************
CODEC=g.723.1


include ../../build/Makefile_test.init




#Set the build specific variables

ifeq ($(TGT_OS),ELINUX)
	CFLAGS    +=
endif

ifeq ($(TGT_OS),UNIX)
     CFLAGS    +=    -DG723_DEBUG_PRINT
endif

ifeq ($(TOOL_CHAIN),RVDS)
	CFLAGS  +=  --inline --autoinline --forceinline
endif

ifeq ($(TGT_ENDIAN),BIG)

 CFLAGS +=  -DG723_BIG_ENDIAN
endif

ifeq ($(TOOL_CHAIN),RVDS)
ifeq ($(TGT_OS),VRTX)
   AFLAGS +=   --debug --keep
   CFLAGS  += --pointer_alignment=2 --min_array_alignment=4\
		            --debug -Oldrd\
		            --apcs=inter --memaccess=-UL41 -D__arm -D__NORM_ -DG723_ARM11_VERSION -DARM_OPTS -DDEBUG -DG723_BIG_ENDIAN -DOS_VRTX

   LFLAGS += --be32 --map --ro_base=0x90000000

endif

endif
ifeq ($(TGT_OS),SYMBIAN)
  C_OBJS = $(APP_DIR)/common/log_api.o
endif
ifeq ($(TGT_OS),ELINUX)
  C_OBJS = $(APP_DIR)/common/log_api.o
endif
C_OBJS = $(APP_DIR)/common/strfunc.o


C_DEC_OBJS = $(APP_DIR)/decoder/g723_dectest.o
C_ENC_OBJS = $(APP_DIR)/encoder/g723_enctest.o

ifeq ($(TGT_OS),VRTX)

   ASM_OBJS = ../../common/asm_arm/rt_memory.o
   ASM_OBJS += ../../common/asm_arm/rt_mem_h.o
   ASM_OBJS += ../../common/asm_arm/initcache.o

endif

ifeq ($(TGT_OS),VRTX)
ALL_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(C_ENC_OBJS) $(ASM_OBJS)
DEC_OBJS = $(C_OBJS) $(C_DEC_OBJS) $(ASM_OBJS)
ENC_OBJS = $(C_OBJS) $(C_ENC_OBJS) )$(ASM_OBJS)
else
ALL_OBJS = $(C_OBJS) $(C_ENC_OBJS) $(C_DEC_OBJS)
DEC_OBJS = $(C_OBJS) $(C_DEC_OBJS)
ENC_OBJS = $(C_OBJS) $(C_ENC_OBJS)
endif

#to enable profiling
ifeq ($(TGT_OS),SYMBIAN)
	ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE
	endif
 	ifeq ($(MEMORY_MEASUREMENT),1)
	 	CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
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
else
	ifeq ($(TGT_OS),LERVDS)
		ifeq ($(TIME_PROFILE),1)
			ifeq ($(TGT_CPU),ARM11)
			     CFLAGS += -DTIME_PROFILE_RVDS
			else
			     CFLAGS += -DTIME_PROFILE_RVDS_ARM9
			endif
			CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
		endif
	endif
	ifeq ($(TGT_OS),BERVDS)
		CFLAGS += -DCYCLE_MEASUREMENT
	endif
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
		$(LN) $(INFLAGS) $(DEC_OBJS) $(C_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_DEC) --symbols --list $(PROGRAM_DEC).map  $(LIBRARY_DEC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_DEC)  $(PROGRAM_DEC)
		$(LN) $(INFLAGS) $(ENC_OBJS) $(C_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_ENC) --symbols --list $(PROGRAM_ENC).map  $(LIBRARY_ENC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_ENC)  $(PROGRAM_ENC)

TEST_ARM11_LERVDS:	$(ALL_OBJS)    $(LIBRARY_ENC) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_LERVDS:	$(ALL_OBJS)  $(LIBRARY_ENC) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_BERVDS:	$(ALL_OBJS)    $(LIBRARY_ENC) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_BERVDS:	$(ALL_OBJS)  $(LIBRARY_ENC) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)



TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
#	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
#	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_X86_UNIX	: $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(DEC_OBJS) $(LIBRARY_DEC)
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(ENC_OBJS) $(LIBRARY_ENC)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM_DEC) $(PROGRAM_ENC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
