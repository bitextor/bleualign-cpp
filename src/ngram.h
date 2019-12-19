
#ifndef FAST_BLEUALIGN_NGRAMS_H
#define FAST_BLEUALIGN_NGRAMS_H

#include "util/murmur_hash.hh"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>
#include <type_traits>

#include <boost/unordered_map.hpp>

namespace ngram {

    struct NGram {
      size_t hash;
      size_t size;

      // unigram
      NGram(const std::string &a) {
        // calc hash of a 
        hash = 0;
        compute_hash(a);
        size = 1;
      }

      // bigram
      NGram(const std::string &a, const std::string &b) {
        hash = 0;
        compute_hash(a);
        compute_hash(b);
        size = 2;
      }

      // trigram
      NGram(const std::string &a, const std::string &b, const std::string &c) {
        hash = 0;
        compute_hash(a);
        compute_hash(b);
        compute_hash(c);
        size = 3;
      }

      // quadgram
      NGram(const std::string &a, const std::string &b, const std::string &c, const std::string &d){
        hash = 0;
        compute_hash(a);
        compute_hash(b);
        compute_hash(c);
        compute_hash(d);
        size = 4;
      }

      void compute_hash(const std::string& a){
        hash = util::MurmurHashNative(a.c_str(), a.size(), hash);
      }

    };

    bool operator==(const NGram &a, const NGram &b);
    size_t hash_value(const NGram &ng);

    typedef boost::unordered_map<NGram, size_t> ngram_map;

    class NGramCounter {

    public:

        NGramCounter(unsigned short n);

        ~NGramCounter() {};

        size_t get(const NGram &key) const;

        ngram_map::iterator begin(size_t ngram) {
          return data.at(ngram - 1).begin();
        }

        ngram_map::iterator end(size_t ngram) {
          return data.at(ngram - 1).end();
        }

        void increment(const NGram &ng);

        void process(std::vector<std::string> &tokens);

        inline void
        increment_helper(std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators, unsigned short ngram);

        size_t count_tokens();

        size_t count_frequencies() {
          return total_freq;
        }

        size_t processed() {
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
