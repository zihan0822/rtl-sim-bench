include ../common.mk
FIRRTL_SOURCE := ../chip-designs/rocket20.fir
KSIM := ksim
FIRTOOL := firtool
TEST_ELF := ../workloads/tests/rv64ui-p-add

run: emulator
	./emulator -c $(TEST_ELF) 

emulator: TestHarness.o emulator.cpp
	git apply fesvr.patch
	$(CXX) $(INCLUDES) $(LIBS) -o $@ $^ -lfesvr

TestHarness.o: $(FIRRTL_SOURCE) 
	$(FIRTOOL) --ir-hw --disable-all-randomization $(FIRRTL_SOURCE) -o TestHarness.mlir 
	$(KSIM) TestHarness.mlir -v -o TestHarness.ll --out-header=TestHarness.h --out-driver=emulator.cpp
	llc --relocation-model=dynamic-no-pic -O2 -filetype=obj TestHarness.ll -o $@ 

clean:
	@rm -f TestHarness.* emulator
