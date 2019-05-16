#*****************************************************************************
#
# oggvorbis_dec Test App Makefile
#
#*****************************************************************************
CODEC=oggvorbis_dec



include ../../build/Makefile_test.init

#Set the build specific variables

ifeq ($(TGT_OS),ELINUX)
 ifeq ($(TGT_CPU),ARM11)

	EXTRA_TESTS += -DRELOCATION
	STACK	 +=  -DSTACK_USAGE
	CFLAGS   +=
 endif
 ifeq ($(TIME_PROFILE),1)
    CPU_FREQ = 1000
 	CFLAGS += -DTIME_PROFILE=1 -DCPUFREQ=$(CPU_FREQ)
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

	CFLAGS	+= -DARM_ADS -fy --loose_implicit_cast -g --inline --autoinline --forceinline
    ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE_RVDS
		CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
    endif
endif

ifeq ($(TOOL_CHAIN),RVDS)
  ifeq ($(TGT_OS),ELINUX)

	CFLAGS   += -O3
 endif
endif


C_OBJS  = $(APP_DIR)/ivorbisfile_example.o
ifeq ($(TGT_OS),LERVDS)
 ifeq ($(TIME_PROFILE),1)
  C_OBJS += ../../common/c_src/profile_utils.o
 endif
endif

ifeq ($(RAW_DATA),0)
CFLAGS += -DxRAW_DATA_INPUT
else
CFLAGS += -DRAW_DATA_INPUT
endif

ALL_OBJS = $(C_OBJS)


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
TEST_ARM11_LERVDS:	$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)
TEST_ARM11_BERVDS:
	@echo "--- Build not supporting for test $(CODEC) ---"

TEST_ARM11_ELINUX: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
TEST_ARM11_VRTX:
	@echo "--- Build not supporting for test $(CODEC) ---"
TEST_ARM9_ELINUX:
	@echo "--- Build not supporting for test $(CODEC) ---"

TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)


TEST_ARM9_LERVDS:
	@echo "--- Build not supported for Test App $(CODEC) ---"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<


clean:
	rm -f $(ALL_OBJS) $(PROGRAM)
