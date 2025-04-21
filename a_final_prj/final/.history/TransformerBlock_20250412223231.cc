#include "TransformerBlock.hh"

TransformerBlock::TransformerBlock(int embed_dim, int num_heads, int ff_hidden_dim)
    : embed_dim(embed_dim), num_heads(num_heads), head_dim(embed_dim / num_heads)
{
    assert(embed_dim % num_heads == 0);

    auto init = [](int rows, int cols) {
        vector<vector<float>> mat(rows, vector<float>(cols));
        for (auto& row : mat)
            for (auto& val : row)
                val = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        return mat;
    };

    W_q = init(embed_dim, embed_dim);
    W_k = init(embed_dim, embed_dim);
    W_v = init(embed_dim, embed_dim);
    W_o = init(embed_dim, embed_dim);

    W1 = init(embed_dim, ff_hidden_dim);
    W2 = init(ff_hidden_dim, embed_dim);

    norm1_gamma = vector<float>(embed_dim, 1.0f);
    norm1_beta = vector<float>(embed_dim, 0.0f);
    norm2_gamma = vector<float>(embed_dim, 1.0f);
    norm2_beta = vector<float>(embed_dim, 0.0f);
}

vector<vector<float>> TransformerBlock::forward(const vector<vector<float>>& input) {
    auto attn_out = self_attention(input);
    for (size_t i = 0; i < input.size(); ++i)
        for (size_t j = 0; j < input[0].size(); ++j)
            attn_out[i][j] += input[i][j]; // residual
    auto norm1 = layer_norm(attn_out, norm1_gamma, norm1_beta);

    auto ff_out = feed_forward(norm1);
    for (size_t i = 0; i < norm1.size(); ++i)
        for (size_t j = 0; j < norm1[0].size(); ++j)
            ff_out[i][j] += norm1[i][j]; // residual
    return layer_norm(ff_out, norm2_gamma, norm2_beta);
}

vector<vector<float>> TransformerBlock::linear(const vector<vector<float>>& input, const vector<vector<float>>& weights) {
    int out_dim = weights[0].size();
    int in_dim = weights.size();
    int seq_len = input.size();
    vector<vector<float>> output(seq_len, vector<float>(out_dim, 0.0f));

    for (int i = 0; i < seq_len; ++i)
        for (int j = 0; j < out_dim; ++j)
            for (int k = 0; k < in_dim; ++k)
                output[i][j] += input[i][k] * weights[k][j];

    return output;
}

vector<vector<float>> TransformerBlock::self_attention(const vector<vector<float>>& x) {
    auto Q = linear(x, W_q);
    auto K = linear(x, W_k);
    auto V = linear(x, W_v);

    int seq_len = x.size();
    vector<vector<float>> output(seq_len, vector<float>(embed_dim, 0.0f));

    for (int h = 0; h < num_heads; ++h) {
        int offset = h * head_dim;
        vector<vector<float>> scores(seq_len, vector<float>(seq_len, 0.0f));
        for (int i = 0; i < seq_len; ++i)
            for (int j = 0; j < seq_len; ++j)
                for (int d = 0; d < head_dim; ++d)
                    scores[i][j] += Q[i][offset + d] * K[j][offset + d];

        for (int i = 0; i < seq_len; ++i)
            softmax(scores[i]);

        for (int i = 0; i < seq_len; ++i)
            for (int d = 0; d < head_dim; ++d)
                for (int j = 0; j < seq_len; ++j)
                    output[i][offset + d] += scores[i][j] * V[j][offset + d];
    }

    return linear(output, W_o);
}

vector<vector<float>> TransformerBlock::feed_forward(const vector<vector<float>>& x) {
    auto hidden = linear(x, W1);
    for (auto& row : hidden)
        for (auto& val : row)
            val = std::max(0.0f, val); // ReLU
    return linear(hidden, W2);
}

vector<vector<float>> TransformerBlock::layer_norm(const vector<vector<float>>& x, const vector<float>& gamma, const vector<float>& beta) {
    int seq_len = x.size();
    int dim = x[0].size();
    vector<vector<float>> out(seq_len, vector<float>(dim));

    for (int i = 0; i < seq_len; ++i) {
        float mean = 0.0f, var = 0.0f;
        for (int j = 0; j < dim; ++j) mean += x[i][j];
        mean /= dim;
        for (int j = 0; j < dim; ++j) var += (x[i][j] - mean) * (x[i][j] - mean);
        var /= dim;
        float eps = 1e-5f;
        for (int j = 0; j < dim; ++j)
            out[i][j] = gamma[j] * ((x[i][j] - mean) / std::sqrt(var + eps)) + beta[j];
    }

    return out;
}

void TransformerBlock::softmax(vector<float>& x) {
    float max_val = *max_element(x.begin(), x.end());
    float sum = 0.0f;
    for (float& val : x) {
        val = std::exp(val - max_val);
        sum += val;
    }
    for (float& val : x) val /= sum;
}
