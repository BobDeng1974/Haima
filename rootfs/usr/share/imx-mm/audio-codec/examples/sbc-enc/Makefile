###############################################################################
#
#  Freescale Semiconductor, Inc.
#
#                  This code is the property of Freescale.
#        (C) Copyright 2006 Freescale Semiconductor, Inc. All Rights Reserved.
#
#  History :
#	Date		Author		Version		Description
#	May 2007	r12342c		0.1		Draft version
#       Feb 2008        b02737          0.2             Add RVDS test code
#	Apr 2008	b02737		0.3		Add RVDS MCPS test option
#       Apr 2008        b02737          0.4             Delete redundancy code
#
###############################################################################

###############################################################################
#	This makefile builds SBC encoder test application
#
###############################################################################
include ../../build/Makefile_test.init

CODEC=sbc_enc

INCLUDES += -I../../ghdr/sbc -I./hdr
ifeq ($(TGT_OS),UNIX)
   CFLAGS   += --inline -fomit-frame-pointer -funroll-all-loops
endif

ifeq ($(TGT_OS),SYMBIAN)
   		CFLAGS   +=  -O3 --inline
    ifeq ($(TIME_PROFILE),1)
		CFLAGS += -DTIME_PROFILE		
    endif
endif

ifeq ($(TGT_OS),ELINUX)
   		CFLAGS   +=  -O3 --inline -fomit-frame-pointer -funroll-all-loops
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

ifeq ($(TGT_OS),LERVDS)
    ifeq ($(TIME_PROFILE),1)
	ifeq ($(TGT_CPU),ARM9)
   		CFLAGS   += -DARM9_MIPS_TEST_LERVDS  # used for test ARM9 MIPS on RVDS
	endif
	ifeq ($(TGT_CPU),ARM11)
   		#CFLAGS   += -DARM11_MIPS_TEST_LERVDS  # used for test MIPS on RVDS
   		CFLAGS   += -DARM9_MIPS_TEST_LERVDS  # used for test ARM9 MIPS on RVDS
	endif
    endif
else
   #CFLAGS   += -DRE_ENTRANCY
endif

C_OBJS += $(APP_DIR)/sbc_enc_test_handler.o
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
TEST_X86_UNIX:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM9_LERVDS:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

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


TEST_ARM11_LERVDS:$(ALL_OBJS) $(LIBRARY)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(LIBRARY)

TEST_ARM11_BERVDS:
		@echo "--- Build not supported for library $(CODEC) ---"

TEST_ARM11_ELINUX	: $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
ifneq ($(BUILD),ARM11GSELINUX)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
else
ifeq ($(LIB_TYPE),STATIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY)) $(GSO_LIBS)
endif
ifeq ($(LIB_TYPE),DYNAMIC)
		$(LN) -o $(PROGRAM) $(LFLAGS) $(ALL_OBJS) $(subst $(LIB_DIR)/,,$(LIBRARY))
endif
endif

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(PROGRAM) $(PROGRAM).map main.in


