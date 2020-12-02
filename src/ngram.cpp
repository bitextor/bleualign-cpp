#include "ngram.h"
#include "util/murmur_hash.hh"

#include <string>
#include <vector>
#include <numeric>
#include <algorithm> 

namespace {
  typedef std::vector<std::string>::const_iterator token_iterator;

  size_t increment_helper(std::vector<token_iterator> const &token_iterators,
                   unsigned short ngram,
                   std::vector<ngram::ngram_map> &maps) {
    size_t hash = 0;
    for (unsigned short i = 1; i <= ngram; ++i){
      hash = ngram::get_token_hash(*token_iterators[ngram-i], hash);
      maps[i - 1][hash] += 1;
    }
    return ngram; // number of iterations of the for-loop
  }
}


namespace ngram {
  size_t get_token_hash(const std::string &token, size_t seed){
    return util::MurmurHashNative(token.c_str(), token.size(), seed);
  }

  NGramCounter::NGramCounter(unsigned short n) : ngram_size_(n) {
    data_.resize(n);
  }

  size_t NGramCounter::get(size_t key, unsigned short ngram) const {
    // No need to be fast, only used for tests anyway
    for (ngram_pair const &pair : data_[ngram - 1])
      if (pair.first == key)
        return pair.second;

    return 0;
  }

  void NGramCounter::process(std::vector<std::string> const &tokens) {
    data_.clear();
    data_.resize(ngram_size_);
    
    total_freq_ = 0;
    tokens_processed_ = tokens.size();

    if (tokens.empty())
      return;

    // Set up hash maps
    std::vector<ngram_map> maps(ngram_size_);

    // set-up iterators
    std::vector<::token_iterator> token_iterators(ngram_size_);

    for (unsigned short i = 0; i < ngram_size_; ++i) {
      token_iterators[i] = tokens.begin();
      std::advance(token_iterators[i], i);
    }

    // base
    for (unsigned short i = 1; i < ngram_size_; ++i) {
      total_freq_ += ::increment_helper(token_iterators, i, maps);
    }

    // continue
    while (token_iterators[ngram_size_ - 1] != tokens.end()) {
      total_freq_ += ::increment_helper(token_iterators, ngram_size_, maps);

      for (unsigned short i = 0; i < ngram_size_; ++i)
        ++token_iterators[i];
    }

    // convert maps to sorted vectors
    for (unsigned short i = 0; i < ngram_size_; ++i) {
      data_[i].reserve(maps[i].size());
      std::move(maps[i].begin(), maps[i].end(), std::back_inserter(data_[i]));
      std::sort(data_[i].begin(), data_[i].end()); // sorts by first
    }
  }

  size_t NGramCounter::count_tokens() const {
    return std::accumulate(data_.begin(), data_.end(), 0, [](size_t acc, ngram_vector const &map) {
      return acc + map.size();
    });
  }
}
