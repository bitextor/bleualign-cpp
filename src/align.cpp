
#include "align.h"
#include "scorer.h"
#include "ngram.h"
#include "search.h"
#include "utils/common.h"

#include <algorithm>
#include <math.h>
#include <boost/make_unique.hpp>
#include <vector>
#include <memory>
#include <iomanip>

namespace align {

    void AlignDocument(const utils::DocumentPair& doc_pair, double threshold) {

      utils::matches_vec matches;

      Align(matches, doc_pair.text1translated, doc_pair.text2, threshold);
      WriteAlignedTextToStdout(matches, doc_pair.text1, doc_pair.text2, doc_pair.url1, doc_pair.url2);

    }

    void Align(utils::matches_vec &matches, const std::vector<std::string> &text1translated_doc,
               const std::vector<std::string> &text2_doc, double threshold) {

      std::vector<utils::scoremap> scorelist;
      EvalSents(scorelist, text1translated_doc, text2_doc, 2, 3);
      search::FindMatches(matches, scorelist, text1translated_doc.size(), text2_doc.size(), threshold);
      GapFiller(matches, text1translated_doc, text2_doc, 3, threshold);

    }

    /* given list of test sentences and list of reference sentences, calculate bleu scores */
    void EvalSents(std::vector<utils::scoremap> &scorelist, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, unsigned short ngram_size, size_t maxalternatives) {

      std::vector<ngram::NGramCounter> src_corpus_ngrams;
      std::vector<std::string> text_normalized;

      // count ngrams for each sentence of the source corpus
      for (const std::string &src_sentence : text2_doc) {
        scorer::normalize(text_normalized, src_sentence, "western");
        ngram::NGramCounter counter (ngram_size);
        counter.process(text_normalized);
        src_corpus_ngrams.push_back(counter);
      }

      // for each sentence of the target corpus, compute the bleu score with each sentence of the source
      // keep <maxalternatives> best options
      for (const std::string &trg_sentence : text1translated_doc) {

        // tokenize and count ngrams of the target sentence
        scorer::normalize(text_normalized, trg_sentence, "western");
        ngram::NGramCounter trg_counts(ngram_size);
        trg_counts.process(text_normalized);

        utils::scoremap smap;
        std::vector<int> correct;
        int src_ngram_freq, trg_ngram_freq;

        size_t src_corpus_i = 0;
        for (const ngram::NGramCounter &src_counts : src_corpus_ngrams) {
          float logbleu = 0.0;
          correct.assign(ngram_size, 0);

          ngram::ngram_map::const_iterator map_it;
          // compute sum of precision scores for ngrams of order 1 to <ngram_size>
          for (unsigned short order = 1; order <= ngram_size; ++order) {
            map_it = trg_counts.cbegin(order);
            while (map_it != trg_counts.cend(order)) {
              src_ngram_freq = src_counts.get(map_it->first, order);
              if (src_ngram_freq > 0) {
                trg_ngram_freq = trg_counts.get(map_it->first, order);
                correct.at(order-1) += std::min(src_ngram_freq, trg_ngram_freq);
              }
              ++map_it;
            }
            logbleu += log(correct.at(order-1)) - log(std::max<int>(trg_counts.processed() - order + 1, 0));
          }

          // apply uniform weights (wn = 1/N)
          logbleu /= ngram_size;
          // brevity penalty
          logbleu += std::min<float>(0, 1 - static_cast<float>(src_counts.processed()) / static_cast<float>(trg_counts.processed()));

          float src2trg_score = exp(logbleu);

          if (src2trg_score > 0) {
            // calculate bleu score in reverse direction
            logbleu = 0.0;
            for (size_t order = 1; order <= ngram_size; ++order) {
              logbleu += log(correct.at(order-1)) - log(std::max<int>(src_counts.processed() - order + 1, 0));
            }
            logbleu /= ngram_size;
            logbleu += std::min<float>(0, 1 - static_cast<float>(trg_counts.processed()) / static_cast<float>(src_counts.processed()));
            float trg2src_score = exp(logbleu);
            float meanscore = (2 * src2trg_score * trg2src_score) / (src2trg_score + trg2src_score);
            smap.insert(utils::scoremap::value_type(meanscore, std::make_pair(src_corpus_i, correct)));
          }
          ++src_corpus_i;
        }

        // keep top N items
        if (smap.size() > maxalternatives) {
          utils::scoremap::iterator rem_it = smap.end();
          std::advance(rem_it, -(maxalternatives));
          smap.erase(smap.begin(), rem_it);
        }

        scorelist.push_back(smap);
      }

    }

    void GapFiller(utils::matches_vec &matched, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, size_t gap_limit, double threshold) {

      // check that matches vector contains only 1:1 matches
      for (auto m: matched) {
        if (!m.first.same() || !m.second.same())
          throw "Inconsistent data in matches!";
      }

      std::unique_ptr<int[]> matches_arr_translated = boost::make_unique<int[]>(text1translated_doc.size());
      std::unique_ptr<int[]> matches_arr_text2 = boost::make_unique<int[]>(text2_doc.size());
      std::fill(matches_arr_translated.get(), matches_arr_translated.get() + text1translated_doc.size(), -1);
      std::fill(matches_arr_text2.get(), matches_arr_text2.get() + text2_doc.size(), -1);


      for (auto m: matched) {
        matches_arr_translated[m.first.from] = m.second.from;
        matches_arr_text2[m.second.from] = m.first.from;
      }

      std::vector<std::string> merged_text_translated;
      utils::vec_pair merged_pos_translated;
      std::vector<std::string> merged_text_text2;
      utils::vec_pair merged_pos_text2;

      for (auto &m: matched) {
        for (int post = 0; post < 2; ++post) {

          if (post == 0) { // pre
            PreGapMergedSentences(merged_text_translated, merged_pos_translated, text1translated_doc,
                                  matches_arr_translated, m.first.from, gap_limit);
            PreGapMergedSentences(merged_text_text2, merged_pos_text2, text2_doc,
                                  matches_arr_text2, m.second.from, gap_limit);
          } else if (post == 1) { // post
            PostGapMergedSentences(merged_text_translated, merged_pos_translated, text1translated_doc,
                                   matches_arr_translated, text1translated_doc.size(), m.first.from, gap_limit);
            PostGapMergedSentences(merged_text_text2, merged_pos_text2, text2_doc,
                                   matches_arr_text2, text2_doc.size(), m.second.from, gap_limit);
          }

          if (merged_text_translated.size() == 1 && merged_text_text2.size() == 1)
            continue;

          std::vector<utils::scoremap> scorelist;
          EvalSents(scorelist, merged_text_translated, merged_text_text2, 2, 3);

          // find max
          float max_val = -1;
          size_t max_pos_translate;
          size_t max_pos_text2;
          for (size_t i = 0; i < scorelist.size(); ++i) {
            if (scorelist.at(i).size() == 0) {
              continue;
            }

            if (scorelist.at(i).rbegin()->first > max_val && scorelist.at(i).rbegin()->first > threshold) {
              max_val = scorelist.at(i).rbegin()->first;
              max_pos_translate = i;
              max_pos_text2 = scorelist.at(i).rbegin()->second.first;
            }
          }

          // update match
          if (max_val != -1) {
            m = utils::match(
                    merged_pos_translated.at(max_pos_translate).first,
                    merged_pos_translated.at(max_pos_translate).second,
                    merged_pos_text2.at(max_pos_text2).first,
                    merged_pos_text2.at(max_pos_text2).second, max_val);
          }


          FillMatches(matches_arr_translated, matches_arr_text2, m);

        }
      }


    }

    void PreGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                               const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                               size_t gap_limit) {

      int start_post = pos - 1;
      while (start_post >= 0) {
        if (matches_arr[start_post] != -1) break;
        if (start_post < signed(pos) - signed(gap_limit) + 1) break;
        --start_post;
      }

      ProduceMergedSentences(merged_text, merged_pos, docs, start_post + 1, pos, gap_limit, true);

    }


    void PostGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr,
                                size_t matches_arr_size, size_t pos, size_t gap_limit) {

      int start_post = pos + 1;
      while (start_post < signed(matches_arr_size)) {
        if (matches_arr[start_post] != -1) break;
        if (start_post > signed(pos) + signed(gap_limit) - 1) break;
        ++start_post;
      }

      ProduceMergedSentences(merged_text, merged_pos, docs, pos, start_post - 1, gap_limit, false);

    }


    void ProduceMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, size_t from, size_t to, size_t limit,
                                bool reverse) {
      merged_text.clear();
      merged_pos.clear();
      std::stringstream ss;

      if (reverse) {
        size_t limited_end = std::min(limit, to - from + 1);
        for (size_t i = 0; i < limited_end; ++i) {
          ss.str("");
          for (size_t j = 0; j <= i; ++j) {
            ss << docs.at(to - i + j) << " ";
          }

          merged_text.push_back(ss.str());
          merged_pos.push_back(std::make_pair(to - i, to));
        }

      } else {
        size_t limited_end = std::min(limit, to - from + 1);
        for (size_t i = 0; i < limited_end; ++i) {
          ss.str("");
          for (size_t j = 0; j <= i; ++j) {
            ss << docs.at(from + j) << " ";
          }

          merged_text.push_back(ss.str());
          merged_pos.push_back(std::make_pair(from, from + i));
        }

      }

    }


    void FillMatches(std::unique_ptr<int[]> &arr1, std::unique_ptr<int[]> &arr2, utils::match m) {
      for (size_t i = m.first.from; i <= m.first.to; ++i) {
        arr1[i] = m.second.from;
      }

      for (size_t i = m.second.from; i <= m.second.to; ++i) {
        arr2[i] = m.first.from;
      }
    }

    void WriteAlignedTextToStdout(const utils::matches_vec &matches,
                                const std::vector<std::string> &text1_doc,
                                const std::vector<std::string> &text2_doc,
                                const std::string& url1,
                                const std::string& url2) {
      for (auto m: matches) {
        std::cout << url1 << "\t" << url2 << "\t";
        for (size_t i = m.first.from; i < m.first.to; ++i) {
          std::cout << text1_doc[i] << ' ';
        }
        std::cout << text1_doc[m.first.to] << "\t";

        for (size_t i = m.second.from; i < m.second.to; ++i) {
          std::cout << text2_doc[i] << ' ';
        }
        std::cout << text2_doc[m.second.to] << "\t";
        std::cout << std::fixed << std::setprecision(6) << m.score << "\n";
      }
    }

} // namespace align
