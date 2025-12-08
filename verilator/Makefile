include ../common.mk
VERILATOR_ROOT ?= /usr/local/share/verilator

all: emulator-verilated
	
emulator-verilated: libVTestHarness 
	@cd $(BUILD) && \
		g++ -O3 -o emulator $(ROOT_DIR)/verilator/emulator.cc \
			-I$(VERILATOR_ROOT)/include -I$(VERILATOR_ROOT)/include/vltstd -Iobj_dir \
			-Lobj_dir -lfesvr -lVTestHarness -lverilated

verilator: $(VERILOG_SOURCE)
	@mkdir -p $(BUILD)
	@-cd $(BUILD) && \
		(time verilator --sv --cc $^ ../resources/verilog/*.v ../resources/cxx/SimDTM.cc \
			-O3 -x-assign fast -x-initial fast \
			-CFLAGS -O3 \
			-top-module $(TOP_MODULE)) 2> $(GENERATION_TIME_LOG)

libVTestHarness: verilator
	@cd $(BUILD) && \
		(CCACHE_DISABLE=1 time make -C obj_dir -f VTestHarness.mk libVTestHarness) 2> $(COMPILE_TIME_LOG) 

clean:
	@rm -rf $(BUILD)