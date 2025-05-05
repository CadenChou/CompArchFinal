#!/bin/bash

L1_SIZES=("16kB" "32kB" "64kB")
L2_SIZES=("128kB" "256kB" "512kB")

BINARY="fnl/a_final_prj/a_final_prj/final/transformer_run.riscv"

OUTPUT_DIR="results"
mkdir -p "$OUTPUT_DIR"

for L1 in "${L1_SIZES[@]}"; do
  for L2 in "${L2_SIZES[@]}"; do
    echo "Rn runing L1=$L1, L2=$L2"

    RUN_DIR="${OUTPUT_DIR}/L1_${L1}_L2_${L2}"
    mkdir -p "$RUN_DIR"

    CMD="build/RISCV/gem5.debug configs/deprecated/example/se.py \
      -c $BINARY \
      --cpu-type=TimingSimpleCPU \
      --caches --l2cache \
      --l1d_size=$L1 --l1i_size=$L1 --l2_size=$L2 \
      --mem-size=512MB"

    echo "--Curently doing---: $CMD"
    eval "$CMD" > "${RUN_DIR}/stdout.txt" 2> "${RUN_DIR}/stderr.txt"

    if [ -f "m5out/stats.txt" ]; then
      mv "m5out/stats.txt" "${RUN_DIR}/stats.txt"
    else
      echo "-----ERROR----------- stats.txt not found for L1=$L1 L2=$L2"
      cat "${RUN_DIR}/stderr.txt"
    fi
  done
done