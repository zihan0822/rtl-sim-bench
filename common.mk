RISCV_DIR := $(shell pwd)/riscv
RISCV_TARGET := riscv64-unknown-elf
CXXFLAGS := -O3 -std=c++17
INCLUDES := -I$(RISCV_DIR)/include
LIBS := -L$(RISCV_DIR)/lib -Wl,-rpath,$(RISCV_DIR)/lib

.PHONY: riscv clean

riscv:
	@mkdir -p $(RISCV_DIR)
	@cd riscv-fesvr && mkdir -p build && cd build && \
		../configure --prefix=$(RISCV_DIR) --target=$(RISCV_TARGET) && make install