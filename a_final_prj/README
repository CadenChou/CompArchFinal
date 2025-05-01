# Setup:
- Install RISC-V compiler
  - Run the following:
    - sudo apt update
    - sudo apt install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
  
  


# How to run
Non-MT RISCV
cd a_final_prj; make ARCH=riscv; cd ..; build/RISCV/gem5.debug configs/deprecated/example/se.py -c ./a_final_prj/transformer_run.riscv --cpu-type=O3CPU --num-cpus=1 --caches

MT RISCV
cd a_final_prj; make MODE=mt ARCH=riscv; cd ..; build/RISCV/gem5.debug configs/deprecated/example/se.py -c ./a_final_prj/transformer_run.riscv --cpu-type=O3CPU --num-cpus=3 --caches

X86 Version
cd a_final_prj; make MODE=mt MODE=x86; cd ..; build/X86/gem5.opt configs/deprecated/example/se.py -c ./a_final_prj/transformer_run.x86 --cpu-type=O3CPU --num-cpus=3 --caches

