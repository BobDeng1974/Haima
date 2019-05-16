#*****************************************************************************
#
# bmp_decTest App Makefile
#
#*****************************************************************************
CODEC=bmp_dec



include ../../build/Makefile_test.init

#Set the build specific variables

ifeq ($(TGT_OS),VRTX)
AFLAGS += --debug --keep --cpu=ARM1136J-S
 CFLAGS += --pointer_alignment=1 --min_array_alignment=4 --dwarf2 -DOS_VRTX -DBIG_ENDIAN --debug -O0 \
	     -Otime --apcs=inter --memaccess=-UL41
 LFLAGS += --be32 --map --list=r.map
endif

ifeq ($(TGT_OS),ELINUX)
	ifeq ($(TIME_PROFILE),1)

	CFLAGS += -DTIME_PROFILE=1

	endif


	ifeq ($(TGT_CPU), ARM9)
	  CFLAGS += -O3 -fomit-frame-pointer
	endif
endif

ifeq ($(TGT_OS),SYMBIAN)
	ifeq ($(TIME_PROFILE),1)

	CFLAGS += -DTIME_PROFILE=1

	endif
	ifeq ($(MEMORY_MEASUREMENT),1)
	    		CFLAGS += -DTIME_PROFILE -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE
   	endif
endif
ifeq ($(TGT_OS),LERVDS)
	CFLAGS   += --inline --autoinline --forceinline
      ifeq ($(TIME_PROFILE),1)

	CFLAGS += -DTIME_PROFILE_RVDS=1
    CFLAGS += -DMEASURE_STACK_USAGE -DMEASURE_HEAP_USAGE

	endif

endif

ifeq ($(TGT_OS),BERVDS)

 AFLAGS += --debug --keep --cpu=ARM1136J-S
 CFLAGS += --pointer_alignment=1 --min_array_alignment=4 --dwarf2  --debug -O0 \
	     -Otime --apcs=inter --memaccess=-UL41
 LFLAGS += --be32 --map --list=r.map
 	ifeq ($(TIME_PROFILE),1)
		CFLAGS +=  -DTIME_PROFILE_RVDS
		COMMON_DIR_C = ../../common/c_src
		C_OBJS += $(COMMON_DIR_C)/profile_utils.o
		INCLUDES += -I./c_src
	endif

endif

C_OBJS = $(APP_DIR)/test_bmp.o
C_OBJS += $(APP_DIR)/log_api.o

ifeq ($(TGT_OS),LERVDS)
 ifeq ($(TIME_PROFILE),1)
  C_OBJS += ../../common/c_src/profile_utils.o
 endif
endif

ifeq ($(FLOAT),HW)
  INCDIR+= -I$(SYS_INCLUDE)
  FLAGS += -ftree-vectorize -ffast-math -fsingle-precision-constant -mfloat-abi=hard -DFHW
  LIBS += --sysroot=/usr/armv7-poky-gnueabi-4.7.2-tool-chain/imx6qsabresd
  PROGRAM := $(PROGRAM)_fhw
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
TEST_ARM11_SYMBIAN: $(ALL_OBJS)  $(LIBRARY)
		$(LN) $(INFLAGS) $(ALL_OBJS)
		$(LN) $(LFLAGS) -o $(PROGRAM) --symbols --list $(PROGRAM).map $(SYMLIB)  $(LIBRARY)
		$(ELFTRAN)   $(PROGRAM)  $(PROGRAM)

TEST_ARM11_LERVDS: $(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_BERVDS:
	@echo "--- Build not supported for library $(CODEC) ---"

TEST_ARM11_ELINUX: $(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM9_LERVDS:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))

TEST_ARM9_BERVDS:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_VRTX:
	@echo "--- Build not supported for library $(CODEC) ---"

TEST_ARM9_VRTX:	$(ALL_OBJS) $(LIBRARY)
#		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM9_ELINUX	: $(ALL_OBJS) $(LIBRARY)
ifneq ($(BUILD),ARM9GSELINUX)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
else
ifeq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY)) $(GSO_LIBS)
endif
ifeq ($(LIB_TYPE),DYNAMIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
endif
endif

TEST_X86_UNIX	:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES)  -c  -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<



clean:
	rm -f $(ALL_OBJS) $(PROGRAM)  $(PROGRAM).map main.in
