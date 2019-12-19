
#ifndef FAST_BLEUALIGN_NGRAMS_H
#define FAST_BLEUALIGN_NGRAMS_H

#include <iostream>
#include <string>
#include <vector>
#include <iterator>

#include <boost/unordered_map.hpp>

namespace ngram {

    size_t get_ngram_hash(const std::string &a);
    size_t get_ngram_hash(const std::string &a, const std::string &b);
    size_t get_ngram_hash(const std::string &a, const std::string &b, const std::string &c);
    size_t get_ngram_hash(const std::string &a, const std::string &b, const std::string &c, const std::string &d);

    typedef boost::unordered_map<size_t, size_t> ngram_map;

    class NGramCounter {

    public:

        NGramCounter(unsigned short n);

        ~NGramCounter() {};

        size_t get(size_t key, unsigned short ngram) const;

        ngram_map::iterator begin(unsigned short ngram) {
          return data.at(ngram - 1).begin();
        }

        ngram_map::iterator end(unsigned short ngram) {
          return data.at(ngram - 1).end();
        }

        void increment(size_t key, unsigned short ngram);

        void process(std::vector<std::string> &tokens);

        inline void
        increment_helper(const std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators, unsigned short ngram);

        size_t count_tokens() const;

        size_t count_frequencies() const {
          return total_freq;
        }

        size_t processed() const {
          return tokens_processed;
        }

    private:

        const unsigned short ngram_size;
        size_t total_freq = 0;
        size_t tokens_processed = 0;
        std::vector<ngram_map> data;

    };

}


#endif //FAST_BLEUALIGN_NGRAMS_H
