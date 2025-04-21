# === Configurable Variables ===
MODE ?= normal
TARGET := transformer_run.riscv

# Always compile for RISC-V
CXX := riscv64-linux-gnu-g++
CXXFLAGS := -std=c++11 -O2 -Wall -static

ifeq ($(MODE), mt)
  SRC := main.cc TransformerBlockMT.cc
  HEADERS := TransformerBlockMT.hh
  CXXFLAGS += -DUSE_MT
else
  SRC := main.cc TransformerBlock.cc
  HEADERS := TransformerBlock.hh
endif

# Always clean first
all: clean $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f transformer_run.riscv

run: $(TARGET)
	@echo "To run in gem5:"
	@echo "./build/RISCV/gem5.debug configs/deprecated/example/se.py -c ./$(TARGET)"

.PHONY: all clean run
