#include <vector>
#include <chrono>
#include "simd_transformer.hh"

int main() {
    int seq_len = 64;
    int embed_dim = 64;
    int num_heads = 4;
    int ff_hidden_dim = 128;

    TransformerBlock transformer(embed_dim, num_heads, ff_hidden_dim);

    std::vector<std::vector<float>> input(seq_len, std::vector<float>(embed_dim, 0.1f));

    auto output = transformer.forward(input);

    return 0;
}
