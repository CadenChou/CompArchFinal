# === Configurable Variables ===
MODE ?= normal
ARCH ?= x86
GEM5_ROOT ?= $(abspath ..)

ifeq ($(ARCH), riscv)
  TARGET := transformer_run.riscv
  CXX := riscv64-linux-gnu-g++
else ifeq ($(ARCH), x86)
  TARGET := transformer_run.x86
  CXX := x86_64-linux-gnu-g++
else
  $(error Invalid ARCH: $(ARCH). Use 'riscv' or 'x86')
endif

# Common compiler flags
CXXFLAGS := -std=c++14 -O2 -Wall -static

# Add m5ops include path if using m5_dump_stats
CXXFLAGS += -I$(GEM5_ROOT)/util/m5

# Source files
ifeq ($(MODE), mt)
  SRC := main.cc TransformerBlockMT.cc
  HEADERS := TransformerBlockMT.hh
  CXXFLAGS += -DUSE_MT
else
  SRC := main.cc TransformerBlock.cc
  HEADERS := TransformerBlock.hh
endif

# === Targets ===
all: clean $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f transformer_run.riscv transformer_run.x86

run: $(TARGET)
	@echo "To run in gem5:"
ifeq ($(ARCH), riscv)
	@echo "./build/RISCV/gem5.debug configs/deprecated/example/se.py -c ./$(TARGET) --cpu-type=O3CPU --num-cpus=1 --caches"
else
	@echo "./build/X86/gem5.opt configs/deprecated/example/se.py -c ./$(TARGET) --cpu-type=O3CPU --num-cpus=4 --caches"
endif

.PHONY: all clean run
