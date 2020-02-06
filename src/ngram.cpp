
#include "ngram.h"
#include "util/murmur_hash.hh"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>

#include <boost/make_unique.hpp>


namespace ngram {
    size_t get_token_hash(const std::string &token, size_t seed){
      return util::MurmurHashNative(token.c_str(), token.size(), seed);
    }

    NGramCounter::NGramCounter(unsigned short n) : ngram_size(n) {
      data.assign(n, ngram_map());
    }

    size_t NGramCounter::get(size_t key, unsigned short ngram) const {
      size_t idx = ngram - 1;
      ngram_map::const_iterator it = data.at(idx).find(key);

      if (it != data.at(idx).cend()) {
        return it->second;
      } else {
        return 0;
      }
    }

    void NGramCounter::increment(size_t key, unsigned short ngram) {
      size_t map_idx = ngram - 1;
      ngram_map::iterator it = data.at(map_idx).find(key);

      if (it != data.at(map_idx).end()) {
        // FOUND
        ++it->second;
      } else {
        // NOT FOUND
        data.at(map_idx)[key] = 1;
      }

      ++total_freq;
    }

    void NGramCounter::process(std::vector<std::string> &tokens) {
      // set-up iterators
      std::unique_ptr<std::vector<std::string>::iterator[]> token_iterators(boost::make_unique<std::vector<std::string>::iterator[]>(ngram_size));
      for (unsigned short i = 0; i < ngram_size; ++i) {
        token_iterators[i] = tokens.begin();
        std::advance(token_iterators[i], i);
      }

      // base
      for (unsigned short i = 0; i < ngram_size - 1; ++i) {
        increment_helper(token_iterators, i + 1);
      }

      // continue
      while (token_iterators[ngram_size - 1] != tokens.end()) {
        increment_helper(token_iterators, ngram_size);

        for (unsigned short i = 0; i < ngram_size; ++i) {
          ++token_iterators[i];
        }
      }

      tokens_processed += tokens.size();

    }

    void NGramCounter::increment_helper(const std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators,
                                        unsigned short ngram) {
      size_t hash = 0;
      for (unsigned short i = 1; i <= ngram; ++i){
        hash = get_token_hash(*token_iterators[ngram-i], hash);
        increment(hash, i);
      }
    }

    size_t NGramCounter::count_tokens() const {
      size_t num = 0;
      for (auto s: data) {
        num += s.size();
      }
      return num;
    }
}
