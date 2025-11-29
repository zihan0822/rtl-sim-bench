include ../common.mk
PAR_LEVELS := 1 2 4
EMULATORS := $(PAR_LEVELS:%=build/par-%/emulator)

all: $(EMULATORS) 

build/par-%/emulator: harness-%
	@time clang++ -O3 -std=c++17 -fno-slp-vectorize -fbracket-depth=1024 -Ifirrtl-sig -Ibuild/par-$* \
		$(ROOT_DIR)/essent-design-harness.cpp -o $@ -lfesvr 

harness-%:
	@mkdir -p build/par-$*
	@cd build/par-$* && \
		ln -s -f $(FIRRTL_SOURCE) design.fir && \
		time $(ROOT_DIR)/repcut-sim/repcut/utils/bin/essent ./design.fir -O0 --parallel $*

clean:
	@rm -rf build