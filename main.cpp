
#include "main.h"
#include "src/align.h"
#include "src/utils/common.h"

#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void Process(float bleu_threshold) {
  utils::DocumentPair doc_pair;
  utils::matches_vec matches;
  std::string line;
  std::vector<std::string> split_line;
  while(getline(std::cin, line)) {
    split_line.clear();
    utils::SplitString(split_line, line, '\t');
    doc_pair.url1 = split_line[0];
    doc_pair.url2 = split_line[1];
    utils::DecodeAndSplit(doc_pair.text1, split_line[2], '\n');
    utils::DecodeAndSplit(doc_pair.text2, split_line[3], '\n');
    utils::DecodeAndSplit(doc_pair.text1translated, split_line[4], '\n');
    align::AlignDocument(doc_pair, bleu_threshold);
    matches.clear();
    std::cout << std::flush;
  }
}

int main(int argc, char *argv[]) {

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("bleu-threshold", po::value<float>()->default_value(0.0f), "BLEU threshold for matched sentences");


  po::variables_map vm;
  po::store(po::parse_command_line(argc, reinterpret_cast<const char *const *>(argv), desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Reads matched documents from stdin, outputs aligned sentences to stdout\n" <<
	    "Tab-separated fields of the input are url1, url2, text1_base64, text2_base64, text1translated_base64\n" <<
	    "Tab-separated fields of the output are url1, url2, sent1, sent2, score\n\n" <<
	    desc << std::endl;
    return 1;
  }

  float bleu_threshold = vm["bleu-threshold"].as<float>();

  Process(bleu_threshold);

  return 0;
}
