include ../common.mk
KSIM ?= ksim
FIRTOOL ?= firtool

all: emulator

emulator: emulator.cpp TestHarness.o 
	cd $(BUILD) && \
	 	patch -p1 $< ../fesvr.patch && \
		$(CXX) $(CXXFLAGS) -o $@ $^ -lfesvr

TestHarness.o: $(FIRRTL_SOURCE) 
	@mkdir -p $(BUILD)
	@cd $(BUILD) && \
		(time $(FIRTOOL) --ir-hw --disable-all-randomization $(FIRRTL_SOURCE) -o TestHarness.mlir && \
			$(KSIM) TestHarness.mlir -v -o TestHarness.ll --out-header=TestHarness.h --out-driver=emulator.cpp) 2> $(GENERATION_TIME_LOG) && \
		(time llc --relocation-model=dynamic-no-pic -filetype=obj -O3 TestHarness.ll -o $@) 2> $(COMPILE_TIME_LOG)

clean:
	@rm -f TestHarness.* emulator
