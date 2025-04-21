#include <iostream>
#include <vector>
#include <chrono>
#include "TransformerBlock.hh"

int main() {
    int seq_len = 4;
    int embed_dim = 8;
    int num_heads = 2;
    int ff_hidden_dim = 16;

    TransformerBlock transformer(embed_dim, num_heads, ff_hidden_dim);

    // Dummy input
    std::vector<std::vector<float>> input(seq_len, std::vector<float>(embed_dim, 0.1f));

    // Start timer
    auto start = std::chrono::high_resolution_clock::now();

    // Run forward pass
    auto output = transformer.forward(input);

    // Stop timer
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Print output
    for (const auto& row : output) {
        for (float val : row)
            std::cout << val << " ";
        std::cout << "\n";
    }

    // Print elapsed time
    std::cout << "Forward pass took " << elapsed.count() << " seconds.\n";

    return 0;
}
