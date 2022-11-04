
#ifndef FAST_BLEUALIGN_ALIGN_H
#define FAST_BLEUALIGN_ALIGN_H


#include "search.h"
#include "utils/common.h"

#include <string>
#include <memory>
#include <vector>

namespace align {

    void AlignDocument(const utils::DocumentPair& doc_pair, double threshold, bool print_sent_hash,
                       int paragraph_id_index = -1);

    void Align(utils::matches_vec &matches, const std::vector<std::string> &text1translated_doc,
               const std::vector<std::string> &text2_doc, double threshold, const bool paragraph_identification);

    void EvalSents(std::vector<utils::scoremap> &scorelist, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, unsigned short ngram_size, size_t maxalternatives);

    void GapFiller(utils::matches_vec &matched, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, size_t gap_limit, double threshold);

    void ProduceMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, size_t from, size_t to, size_t limit,
                                bool reverse = false);

    void PreGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                               const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                               size_t gap_limit);

    void PostGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                                size_t matches_arr_size, size_t gap_limit);

    void FillMatches(std::unique_ptr<int[]> &arr1, std::unique_ptr<int[]> &arr2, utils::match m);

    void WriteAlignedTextToStdout(const utils::matches_vec &matches, const std::vector<std::string> &text1_doc,
                                  const std::vector<std::string> &text2_doc, const std::string& url1, const std::string& url2,
                                  const bool print_sent_hash, const bool paragraph_identification);

    std::vector<std::string> GetParagraphInfo(const std::string &sentence);


} // namespace align

#endif //FAST_BLEUALIGN_ALIGN_H
