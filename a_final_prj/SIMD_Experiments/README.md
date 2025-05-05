# SIMD Portion of CSCI1952y Final

gem5 configs are in the `configs` folder. These contain a cache configuration file and two configurations -- a wide non-SIMD CPU that runs the SIMD program along with a narrow SIMD CPU that runs the normal program.

To compile the programs, run the following in the `code` folder:

Normal: `g++ -O3 -march=x86-64 -mno-sse -mno-avx main.cc simd_transformer.cc -o simd_tf`

SIMD: `g++ -O3 -msse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-avx -mno-avx2 -mno-fma -ftree-vectorize main.cc simd_transformer.cc -o simd_tf`

Note that we must explicitly disable SSSE, SSE4 and above, and AVX, as gem5 does not have full support of these yet. Precompiled binaries for all transformer workloads can be found in `progs`.



We have also tried to use RVV as well as a version of gem5 that supports AVX.

Risc-V Vector has a rapid development pace, so there are lots of roadbumps in cross-compiling binaries and getting them to run in gem5. (gem5 also supports ARM NEON, but we had issues cross-compiling as well).

There are gem5 forks that support AVX, as seen here: https://ieeexplore.ieee.org/document/10415419

However, these forks are not well maintained anymore, and building the gem5 binaries for these forks are difficult. While SSE is a bit less 'modern' than some SIMD ISAs, we believe it still illustrates our hypotheses well.
