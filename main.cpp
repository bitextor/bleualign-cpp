
#include "main.h"
#include "src/align.h"
#include "src/utils/common.h"
#include "src/utils/CompressedWriter.h"
#include "src/utils/logging.h"
#include "src/utils/string_to_float.h"

#include "util/string_piece.hh"
#include "util/file_piece.hh"
#include "util/read_compressed.hh"
#include "util/exception.hh"

#include <iostream>
#include <unordered_set>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


namespace po = boost::program_options;

std::string MungeFilePath(const std::string &filePath)
{
  if (boost::filesystem::exists(filePath)) {
    return filePath;
  }
  else if (boost::filesystem::exists(filePath + ".gz")) {
    return filePath + ".gz";
  }
#ifdef XZ_COMPRESS
  else if (boost::filesystem::exists(filePath + ".xz")) {
    return filePath + ".xz";
  }
#endif
  else {
    UTIL_THROW(util::FileOpenException, "File does not exist");
  }

}

void Process(const utils::Config &cfg) {
  utils::DocumentPair doc_pair;
  util::FilePiece in(MungeFilePath(cfg.documents_path).c_str());
  std::vector<StringPiece> split_line, doc;
  utils::matches_vec matches;
  StringPiece l;
  while(in.ReadLineOrEOF(l)) {
    split_line.clear();
    utils::SplitStringPiece(split_line, l, '\t');
    doc_pair.url1 = utils::PieceToString(split_line[0]);
    doc_pair.url2 = utils::PieceToString(split_line[1]);
    utils::SplitString(doc_pair.text1, utils::b64decode(split_line[2]), '\n');
    utils::SplitString(doc_pair.text2, utils::b64decode(split_line[3]), '\n');
    utils::SplitString(doc_pair.text1translated, utils::b64decode(split_line[4]), '\n');
    align::AlignDocument(doc_pair, cfg.bleu_threshold);
    matches.clear();
  }
}

int main(int argc, char *argv[]) {

  utils::init();

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("documents,d", po::value<std::string>()->required(), "path to the document pairs file. Format: url1 <tab> url2 <tab> text1 <tab> text2 <tab> text1translated")
          ("bleu-threshold", po::value<float>()->default_value(0.0f), "BLEU threshold for matched sentences");


  po::variables_map vm;
  po::store(po::parse_command_line(argc, reinterpret_cast<const char *const *>(argv), desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << desc << std::endl;
    return 1;
  }

  utils::Config cfg;
  cfg.documents_path = vm["documents"].as<std::string>();
  cfg.bleu_threshold = vm["bleu-threshold"].as<float>();

  Process(cfg);

  return 0;
}
