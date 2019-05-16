#*****************************************************************************
#
#  g.711 Codec Makefile
#
#*****************************************************************************
CODEC=g.711

include ../../build/Makefile_test.init

ifeq ($(BIT_MATCH),1)
    CFLAGS += -DBIT_MATCH_WITH_REF
endif

ifeq ($(TGT_OS),ELINUX)
CFLAGS +=  --debug #-DTIME_PROFILE   #-DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
#CFLAGS += -DREENTRANCY_TEST
#LFLAGS += --be32

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
   	ifeq ($(MEMORY_MEASUREMENT),1)
    		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
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


ifeq ($(TGT_OS),VRTX)
AFLAGS += --debug --keep --apcs=inter --memaccess=-UL41
CFLAGS  += --debug -O0 --apcs=inter --memaccess=-UL41 -DOS_VRTX -DBIG_ENDIAN
ifeq ($(TGT_CPU), ARM11)
LDFLAGS += --be32 --ro_base=0x90000000
endif
endif

ifeq ($(TGT_OS),BERVDS)
ifeq ($(TGT_CPU), ARM11)
 AFLAGS +=  --debug --keep --cpu=ARM1136J-S
 CFLAGS +=  --debug --cpu=ARM1136J-S  -O0 -Otime -DBIG_ENDIAN -DRVDS_LIB -DTIME_PROFILE_RVDS -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE #--list
 LFLAGS +=   --be32
else
 AFLAGS +=  --debug --keep --cpu=ARM926EJ-S
 AFLAGS +=  --pd "CRMLBEMEM SETL {TRUE}"
 CFLAGS +=  --debug --cpu=ARM926EJ-S  -O0 -Otime -DBIG_ENDIAN -DRVDS_LIB -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE #--list
 LFLAGS +=   --be32 --ro_base=0x80000000 #--scatter link_nand.lds
endif
endif

ifeq ($(TGT_OS),LERVDS)
 ifeq ($(TGT_CPU),ARM11)
 AFLAGS +=  --debug --keep --cpu=ARM1136J-S
 CFLAGS +=  --debug --cpu=ARM1136J-S  -O0 -Otime #-DTIME_PROFILE_RVDS -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE#--list
 endif
 ifeq ($(TGT_CPU),ARM9)
 AFLAGS +=  --debug --keep --cpu=ARM926EJ-S
 CFLAGS +=  --debug --cpu=ARM926EJ-S  -O0 -Otime #-DTIME_PROFILE_RVDS -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE#--list
 endif
 LFLAGS += #--be32
        ifeq ($(TIME_PROFILE_RVDS),1)
                #CFLAGS  +=  -DTIME_PROFILE_RVDS
                CFLAGS  +=  -DMEASURE_STACK_USAGE
                CFLAGS  +=  -DMEASURE_HEAP_USAGE
        endif
        ifeq ($(DUMMYCALL),1)
                CFLAGS += -DDUMMYCALL
        endif
endif

ifeq ($(TGT_OS),UNIX)
 CFLAGS +=  --debug
 #CFLAGS += -DREENTRANCY_TEST
 LFLAGS += #--be32
endif

ifeq ($(TGT_OS),ELINUX)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o
endif
ifeq ($(TGT_OS),SYMBIAN)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o
endif
ifeq ($(TGT_OS),VRTX)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o

	ASM_OBJS =  ../../common/asm_arm/rt_memory.o
	ASM_OBJS +=	../../common/asm_arm/rt_mem_h.o
	ASM_OBJS +=	../../common/asm_arm/initcache.o
	ASM_OBJS +=	../../common/c_src/profile_utils.o
endif

ifeq ($(TGT_OS),BERVDS)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_OBJS += ../../common/c_src/profile_utils.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o

	ASM_OBJS =  ../../common/asm_arm/rt_memory.o
	ASM_OBJS +=	../../common/asm_arm/rt_mem_h.o
	ASM_OBJS +=	../../common/asm_arm/initcache.o
	ASM_OBJS +=	../../common/c_src/profile_utils.o
endif

ifeq ($(TGT_OS),LERVDS)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_OBJS += ../../common/c_src/profile_utils.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o

	#ASM_OBJS =  ../../common/asm_arm/rt_memory.o
	#ASM_OBJS +=	../../common/asm_arm/rt_mem_h.o
	#ASM_OBJS +=	../../common/asm_arm/initcache.o
	#ASM_OBJS +=	../../common/c_src/profile_utils.o
endif


ifeq ($(TGT_OS),UNIX)
	C_OBJS = $(APP_DIR)/common/process_cmdline.o
	C_ENC_OBJS = $(APP_DIR)/encoder/g711_encode_test.o
	C_DEC_OBJS = $(APP_DIR)/decoder/g711_decode_test.o
endif


ifeq ($(TGT_OS),UNIX)
	ALL_OBJS = $(C_OBJS) $(C_ENC_OBJS) $(C_DEC_OBJS)
else
	ALL_OBJS = $(C_OBJS) $(C_ENC_OBJS) $(C_DEC_OBJS)  $(ASM_OBJS)
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
		$(LN) $(INFLAGS) $(C_DEC_OBJS) $(C_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_DEC) --symbols --list $(PROGRAM_DEC).map  $(LIBRARY_DEC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_DEC)  $(PROGRAM_DEC)
		$(LN) $(INFLAGS) $(C_ENC_OBJS) $(C_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM_ENC) --symbols --list $(PROGRAM_ENC).map  $(LIBRARY_ENC) $(SYMLIB)
		$(ELFTRAN)   $(PROGRAM_ENC)  $(PROGRAM_ENC)

TEST_ARM11_LERVDS: $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
		$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
		$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)
		#$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map $(LIBRARY_DEC) $(SYMLIB)
		#$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map $(LIBRARY_ENC) $(SYMLIB)

TEST_ARM11_BERVDS: $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
		$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
		$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_LERVDS:	$(ALL_OBJS)  $(LIBRARY_ENC) $(LIBRARY_DEC)
		$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
		$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)
		#$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_DEC).map $(LIBRARY_DEC) $(SYMLIB)
		#$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) --info sizes,totals --info libraries --info unused --symbols --map --list $(PROGRAM_ENC).map $(LIBRARY_ENC) $(SYMLIB)

TEST_ARM9_BERVDS:	$(ALL_OBJS)  $(LIBRARY_ENC) $(LIBRARY_DEC)
		$(LN) --map -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(ASM_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
		$(LN) --map -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(ASM_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)
		#$(LN) --map -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(ASM_OBJS) $(C_DEC_OBJS) --symbols --list $(PROGRAM_DEC).map $(LIBRARY_DEC) $(SYMLIB)
		#$(LN) --map -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(ASM_OBJS) $(C_ENC_OBJS) --symbols --list $(PROGRAM_ENC).map $(LIBRARY_ENC) $(SYMLIB)

TEST_ARM11_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
		$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
		$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM9_VRTX:	$(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
#		$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
#		$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))

TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC)) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))
	$(LN) -o $(PROGRAM_DEC)  $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_DEC))
	$(LN) -o $(PROGRAM_ENC)  $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY_ENC))


TEST_X86_UNIX :  $(ALL_OBJS) $(LIBRARY_DEC) $(LIBRARY_ENC)
	    $(LN) -o $(PROGRAM_DEC) $(LFLAGS) $(C_OBJS) $(C_DEC_OBJS) $(LIBRARY_DEC)
	    $(LN) -o $(PROGRAM_ENC) $(LFLAGS) $(C_OBJS) $(C_ENC_OBJS) $(LIBRARY_ENC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM_ENC) $(PROGRAM_DEC) $(PROGRAM_DEC).map $(PROGRAM_ENC).map main.in
