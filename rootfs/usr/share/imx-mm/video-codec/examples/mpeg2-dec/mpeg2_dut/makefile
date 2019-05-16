#*****************************************************************************
#
# Mpeg2 Decoder Dut Makefile
#
#*****************************************************************************
CODEC = mpeg2_dec


include ../../../build/Makefile_test.init

INCLUDES    = -I../../../ghdr -I../../../common/hdr -I./hdr -I../dut_hdr

OBJ_DIR = ./object
DUT_DIR = ./src
BIN_DIR = ../bin

CODEC_LIBRARY=../$(LIBRARY)

DUT_PLATFORM =
ifeq ($(TGT_CPU),ARM9)
  DUT_PLATFORM = arm9
endif
ifeq ($(TGT_CPU),ARM11)
  DUT_PLATFORM = arm11
endif
ifeq ($(TGT_CPU),ARM12)
  DUT_PLATFORM = arm12
endif

ifeq ($(LIB_TYPE),STATIC)
DUT_LIBRARY= $(BIN_DIR)/libDutDecMPG2_$(DUT_PLATFORM).a
else
DUT_LIBRARY= $(BIN_DIR)/libDutDecMPG2_$(DUT_PLATFORM).so
endif

#DUT_TEST_PATH=/nfsroot/zhoupeng/home/mpeg2/ct/


C_DUT_OBJS = 	  $(OBJ_DIR)/dut_mpg2_api_vts.o


all: LIBRARY_COPY makedir DUT_WRAPPER
	@+echo "--- dut wrapper build-all done ---"

makedir:
	@echo
	@echo "Create directory..."
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	@echo

#cp library from release directory to currrent directory
LIBRARY_COPY:
 ifeq ($(LIB_TYPE),STATIC)
	cp -f ../$(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).a  ./
 else
	cp -f ../$(LIB_DIR)/lib_$(CODEC)_$(TGT_CPU_BIN)_$(TGT_OS_BIN).so ./
 endif


ifeq ($(BUILD),ARM11LERVDS)
DUT_WRAPPER: $(C_DUT_OBJS) $(CODEC_LIBRARY)
	$(TOOLS_DIR)armar --create  $(DUT_LIBRARY) $(C_DUT_OBJS) $(CODEC_LIBRARY)
else
DUT_WRAPPER: $(C_DUT_OBJS) $(CODEC_LIBRARY)
#	$(CC) -shared -o $(DUT_LIBRARY) $(LFLAGS) $(C_DUT_OBJS) $(CODEC_LIBRARY)
#	cp $(DUT_LIBRARY) $(DUT_TEST_PATH)
  ifneq ($(LIB_TYPE),STATIC)
	$(CC) -shared -o $(DUT_LIBRARY) $(LFLAGS) $(C_DUT_OBJS) $(subst ../$(LIB_DIR)/,,$(CODEC_LIBRARY))
  else
	$(CC) -shared -o $(DUT_LIBRARY) $(LFLAGS) $(C_DUT_OBJS) $(CODEC_LIBRARY)
  endif
endif

$(OBJ_DIR)/%.o: $(DUT_DIR)/%.c
	$(CC) $(INCLUDES)  -c  -o $@ $<


clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DUT_LIBRARY)


