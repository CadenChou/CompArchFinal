#include <iostream>
#include "TransformerBlockMT.hh"

int main() {
    int seq_len = 4;
    int embed_dim = 8;
    int num_heads = 2;
    int ff_hidden_dim = 16;

    TransformerBlock transformer(embed_dim, num_heads, ff_hidden_dim);

    // Create dummy input: a matrix of shape [seq_len][embed_dim] filled with 0.1
    std::vector<std::vector<float>> input(seq_len, std::vector<float>(embed_dim, 0.1f));

    // Run the forward pass
    auto output = transformer.forward(input);

    // Print the output
    for (const auto& row : output) {
        for (float val : row)
            std::cout << val << " ";
        std::cout << "\n";
    }

    return 0;
}
