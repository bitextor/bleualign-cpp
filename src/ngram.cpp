
#include "ngram.h"
#include "util/murmur_hash.hh"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>

#include <boost/make_unique.hpp>


namespace ngram {
    size_t get_ngram_hash(const std::string& a){
      return util::MurmurHashNative(a.c_str(), a.size());
    }

    size_t get_ngram_hash(const std::string &a, const std::string &b){
      size_t hash = util::MurmurHashNative(a.c_str(), a.size());
      return util::MurmurHashNative(b.c_str(), b.size(), hash);
    }

    size_t get_ngram_hash(const std::string &a, const std::string &b, const std::string &c){
      size_t hash = util::MurmurHashNative(a.c_str(), a.size());
      hash = util::MurmurHashNative(b.c_str(), b.size(), hash);
      return util::MurmurHashNative(c.c_str(), c.size(), hash);
    }

    size_t get_ngram_hash(const std::string &a, const std::string &b, const std::string& c, const std::string& d){
      size_t hash = util::MurmurHashNative(a.c_str(), a.size());
      hash = util::MurmurHashNative(b.c_str(), b.size(), hash);
      hash = util::MurmurHashNative(c.c_str(), c.size(), hash);
      return util::MurmurHashNative(d.c_str(), d.size(), hash);
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
      switch (ngram) {
        case 4:
          increment(get_ngram_hash(*token_iterators[ngram - 4], *token_iterators[ngram - 3],
                                  *token_iterators[ngram - 2], *token_iterators[ngram - 1]), 4);
        case 3:
          increment(get_ngram_hash(*token_iterators[ngram - 3], *token_iterators[ngram - 2],
                                  *token_iterators[ngram - 1]), 3);
        case 2:
          increment(get_ngram_hash(*token_iterators[ngram - 2], *token_iterators[ngram - 1]), 2);
        case 1:
          increment(get_ngram_hash(*token_iterators[ngram - 1]), 1);
          break;
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
