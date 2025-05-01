#include <iostream>
#include <vector>

#ifdef USE_MT
  #include "TransformerBlockMT.hh"
#else
  #include "TransformerBlock.hh"
#endif

int main() {
    int seq_len = 64;
    int embed_dim = 64;
    int ff_hidden_dim = 128;
    int num_heads = 4;

    TransformerBlock transformer(embed_dim, num_heads, ff_hidden_dim);

    std::vector<std::vector<float>> input(seq_len, std::vector<float>(embed_dim, 0.1f));

    auto output = transformer.forward(input);


    for (const auto& row : output) {
        for (float val : row) std::cout << val << " ";
        std::cout << "\n";
    }

    return 0;
}
