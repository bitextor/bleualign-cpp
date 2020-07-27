
#include "src/align.h"
#include "src/utils/common.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void Process(std::istream &in, float bleu_threshold) {
  utils::DocumentPair doc_pair;
  std::string line;
  std::vector<std::string> split_line;

  size_t n = 0;

  while(getline(in, line)) {
    ++n;

    utils::SplitString(split_line, line, '\t');

    // Expect at least 5 (maybe 6) columns
    if (split_line.size() < 5)
      throw std::runtime_error(((std::stringstream) (std::stringstream() << "Not enough fields on line " << n)).str());

    doc_pair.url1 = split_line[0];
    doc_pair.url2 = split_line[1];
    utils::DecodeAndSplit(doc_pair.text1, split_line[2], '\n', true);
    utils::DecodeAndSplit(doc_pair.text2, split_line[3], '\n', true);

    // Processed version of text 1 (i.e. translated to match language text 2)
    utils::DecodeAndSplit(doc_pair.text1translated, split_line[4], '\n', true);
    if (doc_pair.text1.size() != doc_pair.text1translated.size())
      throw std::runtime_error(((std::stringstream) (std::stringstream() 
        << "On line " << n << " column 3 and 5 don't have an equal number of lines ("
        << doc_pair.text1.size() << " vs " << doc_pair.text1translated.size()
        << ")")).str());
    
    // Optionally sixth column with processed version of text 2 (i.e. to better
    // match with the processed version of text 1)
    if (split_line.size() < 6) {
      doc_pair.text2translated = doc_pair.text2;
    } else {
      utils::DecodeAndSplit(doc_pair.text2translated, split_line[5], '\n', true);

      if (doc_pair.text2.size() != doc_pair.text2translated.size())
        throw std::runtime_error(((std::stringstream) (std::stringstream() 
          << "On line " << n << " column 4 and 6 don't have an equal number of lines ("
          << doc_pair.text2.size() << " vs " << doc_pair.text2translated.size()
          << ")")).str());
    }

    align::AlignDocument(doc_pair, bleu_threshold);
    std::cout << std::flush;
  }
}

int main(int argc, char *argv[]) {
  float bleu_threshold = 0.0f;
  std::vector<std::string> filenames;  

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("bleu-threshold", po::value(&bleu_threshold), "BLEU threshold for matched sentences")
          ("input-file", po::value(&filenames));

  po::positional_options_description positional;
  positional.add("input-file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Reads matched documents from input-files or stdin of none specified, outputs aligned sentences to stdout\n" <<
	    "Tab-separated fields of the input are url1, url2, text1_base64, text2_base64, text1translated_base64 [ ,text2translated_base64 ]\n" <<
	    "Tab-separated fields of the output are url1, url2, sent1, sent2, score\n\n" <<
      "Usage: " << argv[0] << " [--help] [--bleu-threshold <threshold>] [<input-file>...]\n\n" <<
	    desc << std::endl;
    return 1;
  }

  if (filenames.empty())
    Process(std::cin, bleu_threshold);
  else
    for (std::string const &filename : filenames) {
      std::ifstream fin(filename);
      Process(fin, bleu_threshold);
    }

  return 0;
}
