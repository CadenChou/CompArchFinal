#include <iostream>
#include <vector>
#include <chrono>

#ifdef USE_MT
  #include "TransformerBlockMT.hh"
#else
  #include "TransformerBlock.hh"
#endif

int main() {
    int seq_len = 8;
    int embed_dim = 64;
    int num_heads = 4;
    int ff_hidden_dim = 128;

    TransformerBlock transformer(embed_dim, num_heads, ff_hidden_dim);

    std::vector<std::vector<float>> input(seq_len, std::vector<float>(embed_dim, 0.1f));

    auto start = std::chrono::high_resolution_clock::now();
    auto output = transformer.forward(input);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    for (const auto& row : output) {
        for (float val : row) std::cout << val << " ";
        std::cout << "\n";
    }

    std::cout << "Forward pass took " << elapsed.count() << " seconds.\n";
    return 0;
}
