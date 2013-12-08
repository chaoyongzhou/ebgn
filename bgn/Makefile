################################################################################
#
# compile:
#   make bgn 
#
################################################################################
#ROOT_DIR = $(shell pwd)
ROOT_DIR = .

MBITS = $(shell getconf LONG_BIT)

LIB_DIR  = $(ROOT_DIR)/lib
OBJ_DIR  = $(ROOT_DIR)/obj
BIN_DIR  = $(ROOT_DIR)/bin
TOOL_DIR = $(ROOT_DIR)/tool

EXT_INC = `xml2-config --cflags`
EXT_LIB = `xml2-config --libs` -lm -lrt -lpthread -lpcre -lz

BGN_SRC  = $(ROOT_DIR)/bgn_src
BGN_INC  = -I $(ROOT_DIR)/bgn_inc
BGN_LIB  = -L $(LIB_DIR) -lbgn

GDB_SRC = $(ROOT_DIR)/gdb_src
GDB_INC = -I $(ROOT_DIR)/gdb_inc
GDB_LIB = -L $(LIB_DIR) -lgdb

GAN_SRC = $(ROOT_DIR)/ganglia_src
GAN_INC = -I $(ROOT_DIR)/ganglia_inc

REL_SRC = $(ROOT_DIR)/rel_src
REL_INC = -I $(ROOT_DIR)/rel_inc

TST_SRC = $(ROOT_DIR)/test
TST_INC =

BYTESIZE   = 8
WORDSIZE   = ${MBITS}
DOUBLESIZE = 64
MATRIX_VECTOR_WIDTH = 32

MEM_BUFF_128B = 128 
MEM_BUFF_512B = 512 
MEM_BUFF_1K   = 1024
MEM_BUFF_8K   = 8192
MEM_BUFF_16K  = 16384 
MEM_BUFF_32K  = 32768 
MEM_BUFF_48K  = 49152 
MEM_BUFF_64K  = 65536 
MEM_BUFF_128K = 131072 
MEM_BUFF_1M   = 1048576
MEM_BUFF_8M   = 8388608
MEM_BUFF_16M  = 16777216
MEM_BUFF_32M  = 33554432
MEM_BUFF_64M  = 67108864
MEM_BUFF_128M = 134217728 


CFILE_SEG_256B = 256
CFILE_SEG_64K  = 65536 
CFILE_SEG_1M   = 1048576
CFILE_SEG_8M   = 8388608

CFILE_SEG_GROUP_02 = 2
CFILE_SEG_GROUP_04 = 4
CFILE_SEG_GROUP_08 = 8
CFILE_SEG_GROUP_16 = 16


FILE_LOG_RECORDS_400 = 400
FILE_LOG_RECORDS_001K  = 1000
FILE_LOG_RECORDS_002K  = 2000
FILE_LOG_RECORDS_004K  = 4000
FILE_LOG_RECORDS_008K  = 8000
FILE_LOG_RECORDS_010K  = 10000
FILE_LOG_RECORDS_020K  = 20000
FILE_LOG_RECORDS_040K  = 40000
FILE_LOG_RECORDS_080K  = 80000
FILE_LOG_RECORDS_100K  = 100000
FILE_LOG_RECORDS_001M  = 1000000
FILE_LOG_RECORDS_002M  = 2000000
FILE_LOG_RECORDS_004M  = 4000000
FILE_LOG_RECORDS_010M  = 10000000
FILE_LOG_RECORDS_020M  = 20000000

CTHREAD_STACK_MAX_SIZE_008K = 8192
CTHREAD_STACK_MAX_SIZE_016K = 16384
CTHREAD_STACK_MAX_SIZE_032K = 32768
CTHREAD_STACK_MAX_SIZE_064K = 65536
CTHREAD_STACK_MAX_SIZE_128K = 131072
CTHREAD_STACK_MAX_SIZE_256K = 262144
CTHREAD_STACK_MAX_SIZE_512K = 524288
CTHREAD_STACK_MAX_SIZE_001M = 1048576
CTHREAD_STACK_MAX_SIZE_002M = 2097152
CTHREAD_STACK_MAX_SIZE_004M = 4194304
CTHREAD_STACK_MAX_SIZE_008M = 8388608

CTHREAD_STACK_GUARD_SIZE_001K = 1024
CTHREAD_STACK_GUARD_SIZE_002K = 2048
CTHREAD_STACK_GUARD_SIZE_004K = 4096
CTHREAD_STACK_GUARD_SIZE_008K = 8192
CTHREAD_STACK_GUARD_SIZE_016K = 16384
CTHREAD_STACK_GUARD_SIZE_032K = 32768
CTHREAD_STACK_GUARD_SIZE_064K = 65536
CTHREAD_STACK_GUARD_SIZE_128K = 131072
CTHREAD_STACK_GUARD_SIZE_256K = 262144
CTHREAD_STACK_GUARD_SIZE_512K = 524288
CTHREAD_STACK_GUARD_SIZE_001M = 1048576
CTHREAD_STACK_GUARD_SIZE_002M = 2097152
CTHREAD_STACK_GUARD_SIZE_004M = 4194304
CTHREAD_STACK_GUARD_SIZE_008M = 8388608

CONN_NUM             = 1
REQ_THREAD_NUM       = 128
RSP_THREAD_NUM       = 128

#DBG_ENCODING_RULE  = 1
#BYTE_ENCODING_RULE = 2

CMACRO = \
         -D_GNU_SOURCE \
         -D_LARGEFILE_SOURCE \
         -D_FILE_OFFSET_BITS=64 \
         -DSWITCH_ON=1 \
         -DSWITCH_OFF=0 \
         -DLICENSE_SWITCH=SWITCH_OFF \
         -DTASK_GANGLIA_SWITCH=SWITCH_ON \
         -DTASK_SLOW_DOWN_MSEC=50 \
         -DCOROUTINE_SLOW_DOWN_MSEC=50 \
         -DCPU_OVERLOAD_THREASHOLD=70 \
         -DDBG_ENCODING_RULE=1 \
         -DBYTE_ENCODING_RULE=2 \
         -DCPROC_PACKAGE_SWITCH=SWITCH_ON \
         -DSTATIC_MEM_TYPE_CHECK_SWITCH=SWITCH_OFF \
         -DMATRIXR_PATCH_SWITCH=SWITCH_ON \
         -DMATRIXR_PATCH_02_SWITCH=SWITCH_ON \
         -DWORDSIZE=${WORDSIZE} \
         -DDOUBLESIZE=${DOUBLESIZE} \
         -DBYTESIZE=${BYTESIZE} \
         -DCROUTINE_SUPPORT_COROUTINE_SWITCH=SWITCH_OFF \
         -DCROUTINE_SUPPORT_CTHREAD_SWITCH=SWITCH_ON \
         -DTASK_REQ_SHORTCUT=SWITCH_OFF \
         -DTASK_FWD_SHORTCUT=SWITCH_ON \
         -DCROUTINE_STACK_MAX_SIZE=${CTHREAD_STACK_MAX_SIZE_064K} \
         -DCROUTINE_STACK_GUARD_SIZE=${CTHREAD_STACK_GUARD_SIZE_004K} \
         -DCTHREAD_SET_CORE_SWITCH=SWITCH_OFF \
         -DTASK_THREAD_SPOOL_LOCK=${TASK_THREAD_SPOOL_CMUTEX} \
         -DTASK_REQ_HANDLE_THREAD_SWITCH=SWITCH_ON \
         -DTASK_REQ_DECODE_THREAD_SWITCH=SWITCH_ON \
         -DTASK_RSP_DECODE_THREAD_SWITCH=SWITCH_ON \
         -DTASK_FWD_DECODE_THREAD_SWITCH=SWITCH_ON \
         -DTASK_REQ_DECODE_AND_HANDLE_THREAD_SWITCH=SWITCH_OFF \
         -DTASK_FWD_DECODE_AND_HANDLE_THREAD_SWITCH=SWITCH_OFF \
         -DCBASE64_ENCODE_SWITCH=SWITCH_OFF \
         -DTASK_ENCODING_RULE=BYTE_ENCODING_RULE \
         -DMATRIX_VECTOR_WIDTH=${MATRIX_VECTOR_WIDTH} \
         -DCSOCKET_SOSNDBUFF_SIZE=${MEM_BUFF_64K} \
         -DCSOCKET_SORCVBUFF_SIZE=${MEM_BUFF_64K} \
         -DCSOCKET_CRBUFF_SIZE=${MEM_BUFF_128M} \
         -DCSOCKET_SO_KEEPALIVE_SWITCH=SWITCH_ON \
         -DCSOCKET_TCP_KEEPIDLE_NSEC=10 \
         -DCSOCKET_TCP_KEEPINTVL_NSEC=5 \
         -DCSOCKET_TCP_KEEPCNT_TIMES=3 \
         -DCSOCKET_SO_SNDTIMEO_NSEC=15 \
         -DCSOCKET_SO_RCVTIMEO_NSEC=15 \
         -DCSOCKET_HEARTBEAT_INTVL_NSEC=10 \
         -DCSOCKET_CNODE_NUM=${CONN_NUM} \
         -DTASK_REPORT_MAX_NUM=16 \
         -DTASK_DEFAULT_LIVE_NSEC=120 \
         -DRANK_HEARTBEAT_ALL_SWITCH=SWITCH_OFF \
         -DRANK_HEARTBEAT_FASTDEC_SWITCH=SWITCH_ON \
         -DRANK_HEARTBEAT_FWD_SWITCH=SWITCH_OFF \
         -DRANK_HEARTBEAT_NODE_SWITCH=SWITCH_OFF \
         -DCLOAD_HEARTBEAT_INTVL_NSEC=10 \
         -DRANK_HEARTBEAT_FASTDEC_INTVL_NSEC=10 \
         -DCLOAD_STAT_UPDATE_INTVL_NSEC=3 \
         -DFILE_SEG_MAX_SIZE=${CFILE_SEG_1M} \
         -DFILE_SEG_GROUP_SIZE=${CFILE_SEG_GROUP_04} \
         -DFILE_LOG_MAX_RECORDS=${FILE_LOG_RECORDS_010M} \
         -DFILE_LOG_NAME_WITH_DATE_SWITCH=SWITCH_OFF \
         -DCBGT_BASED_ON_HSDFS_SWITCH=SWITCH_ON \
         -DCFUSE_SUPPORT_SWITCH=SWITCH_OFF \

# C Language
CC       = /usr/bin/gcc
CLINKER  = /usr/bin/gcc

# C++ Language
#CC       = /usr/bin/g++
#CLINKER  = /usr/bin/g++

AR       = /usr/bin/ar cr

CFLAGS   = -Wall -Wunused -g -m${MBITS} ${CMACRO} -fstack-check
#CFLAGS   = -Wall -Wunused -O3 -m${MBITS} ${CMACRO} -fstack-check
#CFLAGS   = -W -Wunused -ggdb -m${MBITS} ${CMACRO} -O1 -fstack-check
#CFLAGS   = -W -Wunused -ggdb -m${MBITS} ${CMACRO} -O3 -fstack-check
#CFLAGS   = -W -Wunused -m${MBITS} ${CMACRO} -O3 -fstack-check

BGN_OBJS  = $(patsubst $(BGN_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(BGN_SRC)/*.c))
GDB_OBJS  = $(patsubst $(GDB_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(GDB_SRC)/*.c))
REL_OBJS  = $(patsubst $(REL_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(REL_SRC)/*.c))
GAN_OBJS  = $(patsubst $(GAN_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(GAN_SRC)/*.c))
TST_OBJS  = $(patsubst $(TST_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(TST_SRC)/*.c))

usage:
	@echo "usage:"
	@echo "    make libbgn"
	@echo "    make libusr"
	@echo "    make link"
	@echo "    make bgn"
	@echo "    make clean_obj"
	@echo "    make clean_log"
	@echo "    make clean_bin"
	@echo "    make clean_lib"
	@echo "    make clean_trial"
	@echo "    make clean"

libbgn: $(BGN_OBJS)
	$(AR) $(LIB_DIR)/libbgn.a $(BGN_OBJS)

libgdb: $(GDB_OBJS)
	$(AR) $(LIB_DIR)/libgdb.a $(GDB_OBJS)

bgn: libbgn libgdb $(REL_OBJS) $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/bgn $(REL_OBJS) $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(GAN_INC) $(REL_INC) $(EXT_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

link: $(REL_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/bgn $(REL_OBJS) $(BGN_INC) $(GDB_INC) $(EXT_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_csys: libbgn libgdb obj/crun.o obj/test_csys.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_csys.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_crb: libbgn libgdb obj/crun.o obj/test_crb.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_crb.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_cpgrb: libbgn libgdb obj/crun.o obj/test_cpgrb.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_cpgrb.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_cpgb: libbgn libgdb obj/crun.o obj/test_cpgb.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_cpgb.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_cpgd: libbgn libgdb obj/crun.o obj/test_cpgd.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_cpgd.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_cpgv: libbgn libgdb obj/crun.o obj/test_cpgv.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_cpgv.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_crfsdn: libbgn libgdb obj/crun.o obj/test_crfsdn.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_crfsdn.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_crfsnp: libbgn libgdb obj/crun.o obj/test_crfsnp.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_crfsnp.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_crfs: libbgn libgdb obj/crun.o obj/test_crfs.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_crfs.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

test_chfs: libbgn libgdb obj/crun.o obj/test_chfs.o $(GAN_OBJS)
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test obj/crun.o obj/test_chfs.o $(GAN_OBJS) $(BGN_INC) $(GDB_INC) $(TST_INC) $(GAN_INC) $(BGN_LIB) $(GDB_LIB) $(EXT_LIB)

$(BGN_OBJS): $(OBJ_DIR)/%.o: $(BGN_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(BGN_INC) $(GAN_INC) $(GDB_INC) $(EXT_INC) -o $@	

$(GDB_OBJS): $(OBJ_DIR)/%.o: $(GDB_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(BGN_INC) $(GDB_INC) $(EXT_INC) -o $@

$(REL_OBJS): $(OBJ_DIR)/%.o: $(REL_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(BGN_INC) $(GAN_INC) $(GDB_INC) $(REL_INC) $(EXT_INC) -o $@

$(GAN_OBJS): $(OBJ_DIR)/%.o: $(GAN_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(BGN_INC) $(GAN_INC) $(GDB_INC) $(EXT_INC) -o $@

$(TST_OBJS): $(OBJ_DIR)/%.o: $(TST_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(TST_INC) $(BGN_INC) $(GDB_INC) $(EXT_INC) -o $@

clean_obj:
	-rm $(OBJ_DIR)/*.o

clean_lib:
	-rm $(LIB_DIR)/*.a

clean_log:
	-rm $(BIN_DIR)/*.log

clean_bin:
	-rm $(BIN_DIR)/bgn
	-rm $(BIN_DIR)/test

clean_trial: clean_log clean_bin clean_obj
	
clean: clean_log clean_bin clean_obj clean_lib
