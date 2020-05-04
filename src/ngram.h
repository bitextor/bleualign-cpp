#ifndef FAST_BLEUALIGN_NGRAMS_H
#define FAST_BLEUALIGN_NGRAMS_H

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <unordered_map>

namespace ngram {

    size_t get_token_hash(const std::string &token, size_t seed = 0);

    typedef std::unordered_map<size_t, size_t> ngram_map;

    typedef std::pair<size_t,size_t> ngram_pair;

    typedef std::vector<ngram_pair> ngram_vector;

    class NGramCounter {

    public:

        NGramCounter(unsigned short n);

        ~NGramCounter() {};

        size_t get(size_t key, unsigned short ngram) const;

        ngram_vector::const_iterator cbegin(unsigned short ngram) const{
          return data_.at(ngram - 1).cbegin();
        }

        ngram_vector::const_iterator cend(unsigned short ngram) const {
          return data_.at(ngram - 1).cend();
        }

        void process(std::vector<std::string> const &tokens);

        size_t count_tokens() const;

        size_t count_frequencies() const {
          return total_freq_;
        }

        size_t processed() const {
          return tokens_processed_;
        }

    private:
        const unsigned short ngram_size_;
        size_t total_freq_ = 0;
        size_t tokens_processed_ = 0;
        std::vector<ngram_vector> data_;

    };
}


#endif //FAST_BLEUALIGN_NGRAMS_H
