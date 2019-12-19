
#include "ngram.h"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>

#include <boost/make_unique.hpp>


namespace ngram {

    NGramCounter::NGramCounter(unsigned short n) : ngram_size(n) {
      data.assign(n, ngram_map());
    }

    size_t NGramCounter::get(const NGram &key) const {
      size_t idx = key.size - 1;
      ngram_map::const_iterator it = data.at(idx).find(key);

      if (it != data.at(idx).cend()) {
        return it->second;
      } else {
        return 0;
      }
    }

    void NGramCounter::increment(const NGram &ng) {
      size_t map_idx = ng.size - 1;
      ngram_map::iterator it = data.at(map_idx).find(ng);

      if (it != data.at(map_idx).end()) {
        // FOUND
        ++it->second;
      } else {
        // NOT FOUND
        data.at(map_idx)[ng] = 1;
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

    void NGramCounter::increment_helper(std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators,
                                        unsigned short ngram) {
      switch (ngram) {
        case 4:
          increment(NGram(*token_iterators[ngram - 4], *token_iterators[ngram - 3],
                          *token_iterators[ngram - 2], *token_iterators[ngram - 1]));
        case 3:
          increment(NGram(*token_iterators[ngram - 3], *token_iterators[ngram - 2],
                          *token_iterators[ngram - 1]));
        case 2:
          increment(NGram(*token_iterators[ngram - 2], *token_iterators[ngram - 1]));
        case 1:
          increment(NGram(*token_iterators[ngram - 1]));
          break;
      }
    }

    size_t NGramCounter::count_tokens() {
      size_t num = 0;
      for (auto s: data) {
        num += s.size();
      }
      return num;
    }

    bool operator==(const NGram &a, const NGram &b) {return a.hash == b.hash;}
    size_t hash_value(const NGram &ng) {return ng.hash;}
}
