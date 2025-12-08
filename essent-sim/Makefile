include ../common.mk
ESSENT_OPT_LEVELS := O3
EMULATORS := $(ESSENT_OPT_LEVELS:%=$(BUILD)%/emulator)

all: $(EMULATORS)
	
$(BUILD)%/emulator: harness-%
	@(time g++ $(CXXFLAGS) -I$(ROOT_DIR)/firrtl-sig -I$(BUILD)$* \
		$(ROOT_DIR)/essent-design-harness.cpp -o $@ -lfesvr) 2> $(BUILD)$*/$(COMPILE_TIME_LOG)

harness-%: $(FIRRTL_SOURCE)
	@mkdir -p $(BUILD)$*
	@cd $(BUILD)$* && \
		ln -s -f $(FIRRTL_SOURCE) design.fir && \
		(time $(ROOT_DIR)/essent-sim/essent/utils/bin/essent -$* ./design.fir) 2> $(GENERATION_TIME_LOG) 

clean:
	@rm -rf $(BUILD) 