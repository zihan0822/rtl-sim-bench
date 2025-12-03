include ../common.mk

PATCHED_VERILOG_SOURCE := $(patsubst %.sv, %-patched.sv, $(VERILOG_SOURCE))
.PHONY: patronus btor

all: btor patronus
 	
patronus:
	@make -C patronus emulator

btor: $(PATCHED_VERILOG_SOURCE)
	@mkdir -p $(BUILD)
	@(VERILOG_FILES="$^ patronus/resources/verilog/*" \
	 TOP_MODULE=TestHarness \
	 BTOR_OUTPUT=$(BUILD)TestHarness.btor \
		time yosys patronus/verilog2btor.tcl) 2>$(BUILD)gen-time.log
