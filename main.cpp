
#include "src/align.h"
#include "src/utils/common.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::vector<int> ProcessHeader(std::istream &in, bool print_sent_hash, bool paragraph_identification) {
  std::string line;
  std::vector<std::string> split_line;
  std::vector<std::string> header_values = {"src_url", "trg_url", "src_text", "trg_text", "src_translated", "trg_translated"};
  std::vector<int> header_idxs(header_values.size());
  size_t mandatory_fields = std::min((size_t)5, header_values.size()); // First, mandatory fields
  size_t optional_fields = header_values.size() - mandatory_fields ? header_values.size() >= mandatory_fields : 0;

  std::iota(std::begin(header_idxs), std::end(header_idxs), 0); // [0, header_values.size())

  if (header_values.size() != header_idxs.size()) {
    std::stringstream error;
    error << "Code error: different size for header and idxs values";
    throw std::runtime_error(error.str());
  }
  if (header_values.size() != mandatory_fields + optional_fields) {
    std::stringstream error;
    error << "Code error: incorrect values for mandatory and optional values in header fields";
    throw std::runtime_error(error.str());
  }

  // Read header
  getline(in, line);
  utils::SplitString(split_line, line, '\t');

  // Get indexes of the header
  for (size_t i = 0; i < mandatory_fields + optional_fields; ++i) {
    auto find_result = std::find(split_line.begin(), split_line.end(), header_values[i]);

    if (find_result == std::end(split_line)) {
      if (i < mandatory_fields) {
        // Throw error because we could not find the mandatory field
        std::stringstream error;
        error << "Mandatory field '" << header_values[i] << "' not found in header";
        throw std::runtime_error(error.str());
      } else {
        // Optional field not found
        continue;
      }
    }

    // Assign the idx of the field
    header_idxs[i] = find_result - split_line.begin();
  }

  // Print output header
  std::cout << "src_url\ttrg_url\tsrc_text\ttrg_text\tbleualign_score";

  if (paragraph_identification)
    std::cout << "\tsrc_paragraph_id\ttrg_paragraph_id";

  if (print_sent_hash)
    std::cout << "\tsrc_deferred_hash\ttrg_deferred_hash";

  std::cout << "\n";

  return header_idxs;
}

void Process(std::istream &in, float bleu_threshold, bool print_sent_hash, bool paragraph_identification) {
  utils::DocumentPair doc_pair;
  std::string line;
  std::vector<std::string> split_line;
  std::vector<int> header_idxs = ProcessHeader(in, print_sent_hash, paragraph_identification);

  size_t n = 0;

  while(getline(in, line)) {
    ++n;

    utils::SplitString(split_line, line, '\t');

    // Expect at least 5 (maybe 6) columns
    if (split_line.size() < 5) {
      std::stringstream error;
      error << "Not enough fields on line " << n;
      throw std::runtime_error(error.str());
    }

    doc_pair.url1 = split_line[header_idxs[0]];
    doc_pair.url2 = split_line[header_idxs[1]];
    utils::DecodeAndSplit(doc_pair.text1, split_line[header_idxs[2]], '\n', true);
    utils::DecodeAndSplit(doc_pair.text2, split_line[header_idxs[3]], '\n', true);

    // Processed version of text 1 (i.e. translated to match language text 2)
    utils::DecodeAndSplit(doc_pair.text1translated, split_line[header_idxs[4]], '\n', true);
    if (doc_pair.text1.size() != doc_pair.text1translated.size()) {
      std::stringstream error;
      error << "On line " << n << " column 3 and 5 don't have an equal number of lines ("
            << doc_pair.text1.size() << " vs " << doc_pair.text1translated.size() << ")";
      throw std::runtime_error(error.str());
    }
    
    // Optionally sixth column with processed version of text 2 (i.e. to better
    // match with the processed version of text 1)
    if (split_line.size() < 6) {
      doc_pair.text2translated = doc_pair.text2;
    } else {
      utils::DecodeAndSplit(doc_pair.text2translated, split_line[header_idxs[5]], '\n', true);

      if (doc_pair.text2.size() != doc_pair.text2translated.size()) {
        std::stringstream error; 
        error << "On line " << n << " column 4 and 6 don't have an equal number of lines ("
              << doc_pair.text2.size() << " vs " << doc_pair.text2translated.size() << ")";
        throw std::runtime_error(error.str());
      }
    }

    align::AlignDocument(doc_pair, bleu_threshold, print_sent_hash, paragraph_identification);
    std::cout << std::flush;
  }
}

int main(int argc, char *argv[]) {
  float bleu_threshold = 0.0f;
  bool print_sent_hash = false;
  bool paragraph_identification = false;
  std::vector<std::string> filenames;  

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("bleu-threshold", po::value(&bleu_threshold), "BLEU threshold for matched sentences")
          ("print-sent-hash", po::bool_switch(&print_sent_hash)->default_value(false), "print Murmurhash hashes of the output sentences")
          ("paragraph-identification", po::bool_switch(&paragraph_identification)->default_value(false), "sentences from input contain tab separated paragraph identification data")
          ("input-file", po::value(&filenames));

  po::positional_options_description positional;
  positional.add("input-file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Reads matched documents from input-files or stdin of none specified, outputs aligned sentences to stdout\n" <<
	    "Tab-separated fields of the input are url1, url2, text1_base64, text2_base64, text1translated_base64 [ ,text2translated_base64 ]\n" <<
	    "Tab-separated fields of the output are url1, url2, sent1, sent2, score [ , murmurhash_text1, murmurhash_text2 ]\n\n" <<
      "Usage: " << argv[0] << " [--help] [--bleu-threshold <threshold>] [--print-sent-hash] [--paragraph-identification] [<input-file>...]\n\n" <<
	    desc << std::endl;
    return 1;
  }

  if (filenames.empty())
    Process(std::cin, bleu_threshold, print_sent_hash, paragraph_identification);
  else
    for (std::string const &filename : filenames) {
      std::ifstream fin(filename);
      Process(fin, bleu_threshold, print_sent_hash, paragraph_identification);
    }

  return 0;
}
