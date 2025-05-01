#pragma once
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <algorithm>


using std::vector;

class TransformerBlock {
public:
    TransformerBlock(int embed_dim, int num_heads, int ff_hidden_dim);

    vector<vector<float>> forward(const vector<vector<float>>& input); // shape: [seq_len][embed_dim]

private:
    int embed_dim;
    int num_heads;
    int head_dim;

    // Weights for attention
    vector<vector<float>> W_q, W_k, W_v, W_o;

    // Weights for feedforward
    vector<vector<float>> W1, W2;

    // LayerNorm parameters (can be learned; here fixed to 1/0 for simplicity)
    vector<float> norm1_gamma, norm1_beta;
    vector<float> norm2_gamma, norm2_beta;

    // Helpers
    vector<vector<float>> linear(const vector<vector<float>>& input, const vector<vector<float>>& weights);
    vector<vector<float>> self_attention(const vector<vector<float>>& x);
    vector<vector<float>> feed_forward(const vector<vector<float>>& x);
    vector<vector<float>> layer_norm(const vector<vector<float>>& x, const vector<float>& gamma, const vector<float>& beta);
    float dot(const vector<float>& a, const vector<float>& b);
    void softmax(vector<float>& x);
    vector<vector<float>> transpose(const vector<vector<float>>& mat);
};
