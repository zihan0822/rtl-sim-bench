include ../common.mk
PAR_LEVELS := 1 2 4
EMULATORS := $(PAR_LEVELS:%=$(BUILD)par-%/emulator)

all: $(EMULATORS) 

$(BUILD)par-%/emulator: harness-%
	@cd $(BUILD)par-$* && \
		(time clang++ -O3 -std=c++17 -fno-slp-vectorize -fbracket-depth=1024 -I$(ROOT_DIR)/firrtl-sig -I. \
			$(ROOT_DIR)/essent-design-harness.cpp -o emulator -lfesvr) 2> $(COMPILE_TIME_LOG)

harness-%: $(FIRRTL_SOURCE) 
	@mkdir -p $(BUILD)/par-$*
	@cd $(BUILD)/par-$* && \
		ln -s -f $(FIRRTL_SOURCE) design.fir && \
		(time $(ROOT_DIR)/repcut-sim/repcut/utils/bin/essent ./design.fir -O0 --parallel $*) 2> $(GENERATION_TIME_LOG)

clean:
	@rm -rf $(BUILD) 