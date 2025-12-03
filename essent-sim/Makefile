include ../common.mk
ESSENT_OPT_LEVELS := O3
EMULATORS := $(ESSENT_OPT_LEVELS:%=$(BUILD)%/emulator)

all: $(EMULATORS)
	
$(BUILD)%/emulator: harness-%
	@(time g++ $(CXXFLAGS) -I$(ROOT_DIR)/firrtl-sig -I$(BUILD)$* \
		$(ROOT_DIR)/essent-design-harness.cpp -o $@ -lfesvr) > $(BUILD)$*/compile-time.log 2>&1

harness-%: $(FIRRTL_SOURCE)
	@mkdir -p $(BUILD)$*
	@cd $(BUILD)$* && \
		ln -s -f $(FIRRTL_SOURCE) design.fir && \
		(time $(ROOT_DIR)/essent-sim/essent/utils/bin/essent -$* ./design.fir) > gen-time.log 2>&1

clean:
	@rm -rf $(BUILD) 