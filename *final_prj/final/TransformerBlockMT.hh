#pragma once
#include <vector>
#include <thread>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <algorithm>

using std::vector;

class TransformerBlock {
public:
    // Constructor: initialize embedding dimensions, number of heads, and FFN hidden size
    TransformerBlock(int embed_dim, int num_heads, int ff_hidden_dim);

    // Run the transformer block forward pass on a [seq_len][embed_dim] input
    vector<vector<float>> forward(const vector<vector<float>>& input);

private:
    int embed_dim;     // Total embedding size
    int num_heads;     // Number of attention heads
    int head_dim;      // Size of each attention head (embed_dim / num_heads)

    // Attention weight matrices
    vector<vector<float>> W_q;
    vector<vector<float>> W_k;
    vector<vector<float>> W_v;
    vector<vector<float>> W_o;

    // Feedforward layer weights
    vector<vector<float>> W1;
    vector<vector<float>> W2;

    // LayerNorm parameters
    vector<float> norm1_gamma, norm1_beta;
    vector<float> norm2_gamma, norm2_beta;

    // Core component functions
    vector<vector<float>> linear(const vector<vector<float>>& input, const vector<vector<float>>& weights);
    vector<vector<float>> self_attention(const vector<vector<float>>& x);
    vector<vector<float>> feed_forward(const vector<vector<float>>& x);
    vector<vector<float>> layer_norm(const vector<vector<float>>& x, const vector<float>& gamma, const vector<float>& beta);
    void softmax(vector<float>& x);
};
