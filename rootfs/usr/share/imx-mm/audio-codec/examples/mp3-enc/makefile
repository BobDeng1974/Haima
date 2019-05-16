#*****************************************************************************
#
# mp3_enc Test App Makefile
#
#*****************************************************************************
CODEC=mp3_enc

include ../../build/Makefile_test.init

INCLUDES += -I../../ghdr/$(CODEC) -I./hdr

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
            ifeq ($(TGT_CPU),ARM9)
                CFLAGS   += -DTIME_PROFILE_RVDS_ARM9   # used for test ARM9 MIPS on RVDS
            endif
            ifeq ($(TGT_CPU),ARM11)
                CFLAGS  += -DMHZ_MEASURE               # used for test ARM11 MIPS on RVDS
            endif
            ifeq ($(TGT_CPU),ARM12)
                CFLAGS  += -DMHZ_MEASURE_SOC           # used for test ARM12 MIPS on SOC
            endif
        endif
    endif
endif


 ifeq ($(TGT_CPU),ARM12)

       CFLAGS  +=  -DENABLE_MY_FREAD=1 -D__ARM12
 endif


ifeq ($(TGT_OS),ELINUX)
 ifeq ($(TGT_CPU),ARM11)

#	CFLAGS  +=  -DLOG_TIMING=1
 endif
endif

ifeq ($(TGT_OS),VRTX)

#CFLAGS  += -D_DEBUG -DBUILD_INTEGER -D_CONSOLE -DENABLE_LPC -DENABLE_ALL_ENCOPT -DDISABLE_OPT -DBITSTREAM_CORRUPTION_ASSERTS -DWMA_DECPK_BUILD -DWMAAPI_NO_DRM -DWMAAPI_NO_DRM_STREAM -DDISABLE_UES -D_CRT_NON_CONFORMING_SWPRINTFS -D_CRT_SECURE_NO_DEPRECATE
CFLAGS  += -D_DEBUG -DFIXEDPOINT -DARM9E_ASM_SUBBAND -DARM9E_ASM_MS_STEREO -DHIGH_MEM_VERSION -DREENTRANT
AFLAGS += --debug --keep  --apcs=inter --memaccess=-UL41
LFLAGS += --be32 --ro_base=0x90000000

endif

C_OBJS = $(APP_DIR)/test_mp3enc.o\
         $(APP_DIR)/wave_functions.o
	#CFLAGS += -DTWO_INSTANCES

ifeq ($(TGT_OS),VRTX)
        C_OBJS +=
        ASM_OBJS =
        ASM_OBJS +=
endif

ifeq ($(TGT_OS),ELINUX)
        #CFLAGS += -DTIME_PROFILE  #for ELINUX
	CFLAGS  +=-DBUILD_ELINUX  -DLOG_TIMING #-DDISCARD_OUTPUT
endif

ifeq ($(BUILD),UNIX)
	CFLAGS  += -DUNIX
endif

ifeq ($(BUILD),ARM11LERVDS)
	CFLAGS  +=
	#CFLAGS += -DMHZ_MEASURE  #for RVDS
	CFLAGS   += -g --loose_implicit_cast  #-Wabdfrvy #--inline --autoinline --forceinline
endif

ifeq ($(BUILD),ARM11BERVDS)
	CFLAGS  +=

	ifeq ($(TGT_ENDIAN),BIG)
		CFLAGS  +=  -DENABLE_BIG_ENDIAN
	endif

	CFLAGS   += -g --loose_implicit_cast  #-Wabdfrvy #--inline --autoinline --forceinline
endif



ALL_OBJS = $(C_OBJS) $(ASM_OBJS)

ifeq ($(TGT_CPU),ARM12)
ifneq "$(TGT_OS)" "ELINUX"
  C_OBJS += ../../common/c_src/retarget_Cortex.o
  ASM_OBJS += ../../common/asm_arm/Init_Cortex.o \
              ../../common/asm_arm/InitCache_Cortex.o \
              ../../common/asm_arm/Vectors.o
endif
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
TEST_ARM9_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM12_LERVDS:      $(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


TEST_ARM11_BERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_VRTX:
	@echo "--- Build not supporting for library $(CODEC) ---"w


TEST_ARM9_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))

TEST_ARM12_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))


TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


%.o: %.c
	$(CC) -W $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM)





