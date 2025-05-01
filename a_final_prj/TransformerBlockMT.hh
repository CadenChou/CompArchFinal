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
    TransformerBlock(int embed_dim, int num_heads, int ff_hidden_dim);

    vector<vector<float>> forward(const vector<vector<float>>& input);

private:
    int embed_dim;
    int num_heads;
    int head_dim;

    vector<vector<float>> W_q;
    vector<vector<float>> W_k;
    vector<vector<float>> W_v;
    vector<vector<float>> W_o;

    vector<vector<float>> W1;
    vector<vector<float>> W2;

    vector<float> norm1_gamma, norm1_beta;
    vector<float> norm2_gamma, norm2_beta;

    vector<vector<float>> linear(const vector<vector<float>>& input, const vector<vector<float>>& weights);
    vector<vector<float>> linear_single_thread(const vector<vector<float>>& input, const vector<vector<float>>& weights);

    vector<vector<float>> self_attention(const vector<vector<float>>& x);
    vector<vector<float>> feed_forward(const vector<vector<float>>& x);
    vector<vector<float>> layer_norm(const vector<vector<float>>& x, const vector<float>& gamma, const vector<float>& beta);
    void softmax(vector<float>& x);
};
