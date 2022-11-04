
#include "src/align.h"
#include "src/utils/common.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::vector<std::string> GetMandatoryHeaderFields(const std::vector<std::string> &split_metadata_headers) {
  std::vector<std::string> header_mandatory_values = {"src_url", "trg_url", "src_text", "trg_text", "src_translated"};

  if (split_metadata_headers.size() != 0) {
    header_mandatory_values.push_back("src_metadata");
    header_mandatory_values.push_back("trg_metadata");
  }

  return header_mandatory_values;
}

std::unordered_map<std::string, int> ProcessHeader(std::istream &in, bool print_sent_hash,
                                                   const std::vector<std::string> &split_metadata_headers) {
  std::string line;
  std::vector<std::string> split_line;

  // Read header
  getline(in, line);
  utils::SplitString(split_line, line, '\t');

  std::unordered_map<std::string, int> header;
  std::vector<std::string> header_mandatory_values = GetMandatoryHeaderFields(split_metadata_headers);

  for (size_t i = 0; i < split_line.size(); ++i) {
    // Get all fields
    header[split_line[i]] = i;

    // Check out if it is a mandatory field
    auto find_result = std::find(header_mandatory_values.begin(), header_mandatory_values.end(), split_line[i]);

    if (find_result != std::end(header_mandatory_values)) {
      header_mandatory_values.erase(find_result);
    }
  }

  if (header_mandatory_values.size() != 0) {
    // Not all mandatory fields were provided
    std::stringstream error;

    error << "Mandatory field '" << header_mandatory_values[0] << "' not found in header";

    throw std::runtime_error(error.str());
  }

  // Print output header
  std::cout << "src_url\ttrg_url\tsrc_text\ttrg_text\tbleualign_score";

  if (print_sent_hash)
    std::cout << "\tsrc_deferred_hash\ttrg_deferred_hash";

  for (const std::string &metadata_header_field : split_metadata_headers) {
    std::cout << "\tsrc_" << metadata_header_field << "\ttrg_" << metadata_header_field;
  }

  std::cout << "\n";

  return header;
}

void Process(std::istream &in, float bleu_threshold, bool print_sent_hash, std::string paragraph_id_header,
             std::string metadata_headers) {
  utils::DocumentPair doc_pair;
  std::string line;
  std::vector<std::string> split_line;
  std::vector<std::string> split_metadata_headers;

  utils::SplitString(split_metadata_headers, metadata_headers, ',');

  std::unordered_map<std::string, int> header_idxs = ProcessHeader(in, print_sent_hash, split_metadata_headers);
  std::vector<std::string> header_mandatory_fields = GetMandatoryHeaderFields(split_metadata_headers);

  size_t n = 0;
  size_t columns = 0;
  bool metadata = split_metadata_headers.size() != 0 ? true : false;
  int paragraph_id_index = -1;

  if (!paragraph_id_header.empty()) {
    // Paragraph identification checks

    if (!metadata) {
      std::stringstream error;
      error << "Paragraph identification header field is set but no metadata header fields were provided";
      throw std::runtime_error(error.str());
    }

    auto find_result = std::find(split_metadata_headers.begin(), split_metadata_headers.end(), paragraph_id_header);

    if (find_result == std::end(split_metadata_headers)) {
      std::stringstream error;
      error << "Paragraph identification header field (" << paragraph_id_header << ") was not found in the metadata fields";
      throw std::runtime_error(error.str());
    }

    // Update index
    paragraph_id_index = find_result - split_metadata_headers.begin();
  }

  while(getline(in, line)) {
    ++n;

    utils::SplitString(split_line, line, '\t');

    if (columns == 0) {
      // Initialize the expected number of fields for all the lines
      columns = split_line.size();
    }

    // Expect at least 5 (maybe 6 or more if metadata is present) columns
    if (split_line.size() < header_mandatory_fields.size()) {
      std::stringstream error;
      error << "Not enough fields on line " << n << " mandatory header fields are:";

      for (const std::string &field : header_mandatory_fields) {
        error << " " << field;
      }

      throw std::runtime_error(error.str());
    }
    // Check that the number of fields is the expected, since all the lines should contain the same number of fields
    if (columns != split_line.size()) {
      std::stringstream error;
      error << "Different number of fields obtained on line " << n;
      throw std::runtime_error(error.str());
    }

    doc_pair.url1 = split_line[header_idxs["src_url"]];
    doc_pair.url2 = split_line[header_idxs["trg_url"]];
    utils::DecodeAndSplit(doc_pair.text1, split_line[header_idxs["src_text"]], '\n', true);
    utils::DecodeAndSplit(doc_pair.text2, split_line[header_idxs["trg_text"]], '\n', true);

    // Process metadata, if provided
    if (metadata) {
      utils::DecodeAndSplit(doc_pair.text1metadata, split_line[header_idxs["src_metadata"]], '\n', true);
      utils::DecodeAndSplit(doc_pair.text2metadata, split_line[header_idxs["trg_metadata"]], '\n', true);

      if (doc_pair.text1.size() != doc_pair.text1metadata.size()) {
        std::stringstream error;
        error << "On line " << n << " column " << header_idxs["src_text"] + 1 << " and "
              << header_idxs["src_metadata"] + 1 << " don't have an equal number of lines "
              << "(" << doc_pair.text1.size() << " vs " << doc_pair.text1metadata.size() << ")";
        throw std::runtime_error(error.str());
      }
      if (doc_pair.text2.size() != doc_pair.text2metadata.size()) {
        std::stringstream error;
        error << "On line " << n << " column " << header_idxs["trg_text"] + 1 << " and "
              << header_idxs["trg_metadata"] + 1 << " don't have an equal number of lines "
              << "(" << doc_pair.text2.size() << " vs " << doc_pair.text2metadata.size() << ")";
        throw std::runtime_error(error.str());
      }
    }

    // Processed version of text 1 (i.e. translated to match language text 2)
    utils::DecodeAndSplit(doc_pair.text1translated, split_line[header_idxs["src_translated"]], '\n', true);
    if (doc_pair.text1.size() != doc_pair.text1translated.size()) {
      std::stringstream error;
      error << "On line " << n << " column " << header_idxs["src_text"] + 1 << " and "
            << header_idxs["src_translated"] + 1 << " don't have an equal number of lines "
            << "(" << doc_pair.text1.size() << " vs " << doc_pair.text1translated.size() << ")";
      throw std::runtime_error(error.str());
    }
    
    // Optionally sixth column with processed version of text 2 (i.e. to better
    // match with the processed version of text 1)
    if (header_idxs.find("trg_translated") == header_idxs.end()) {
      doc_pair.text2translated = doc_pair.text2;
    } else {
      utils::DecodeAndSplit(doc_pair.text2translated, split_line[header_idxs["trg_translated"]], '\n', true);

      if (doc_pair.text2.size() != doc_pair.text2translated.size()) {
        std::stringstream error; 
        error << "On line " << n << " column " << header_idxs["trg_text"] + 1 << " and "
              << header_idxs["trg_translated"] + 1 << " don't have an equal number of lines "
              << "(" << doc_pair.text2.size() << " vs " << doc_pair.text2translated.size() << ")";
        throw std::runtime_error(error.str());
      }
    }

    align::AlignDocument(doc_pair, bleu_threshold, print_sent_hash, paragraph_id_index);
    std::cout << std::flush;
  }
}

int main(int argc, char *argv[]) {
  float bleu_threshold = 0.0f;
  bool print_sent_hash = false;
  std::string metadata_header_fields;
  std::string paragraph_id_header;
  std::vector<std::string> filenames;

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("bleu-threshold", po::value(&bleu_threshold), "BLEU threshold for matched sentences")
          ("print-sent-hash", po::bool_switch(&print_sent_hash)->default_value(false), "print Murmurhash hashes of the output sentences")
          ("metadata-header-fields", po::value(&metadata_header_fields), "language agnostic header fields, comma separated")
          ("paragraph-id-header", po::value(&paragraph_id_header), "metadata header field of the paragraph identification data")
          ("input-file", po::value(&filenames));

  po::positional_options_description positional;
  positional.add("input-file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Reads matched documents from input-files or stdin of none specified, outputs aligned sentences to stdout\n" <<
	    "Tab-separated fields of the input are url1, url2, text1_base64, text2_base64, text1translated_base64 [ , text2translated_base64 ]\n"
      "[ , text1metadata_base64, text2metadata_base64 ]\n" <<
	    "Tab-separated fields of the output are url1, url2, sent1, sent2, score [ , murmurhash_text1, murmurhash_text2 ]\n"
      "[ , metadata1_text1, metadata1_text2 ...] \n\n" <<
      "Usage: " << argv[0] << " [--help] [--bleu-threshold <threshold>] [--print-sent-hash] [--metadata-headers <field1>,... \n"
      "[--paragraph-identification]] [<input-file>...]\n\n" <<
	    desc << std::endl;
    return 1;
  }

  if (filenames.empty())
    Process(std::cin, bleu_threshold, print_sent_hash, paragraph_id_header, metadata_header_fields);
  else
    for (std::string const &filename : filenames) {
      std::ifstream fin(filename);
      Process(fin, bleu_threshold, print_sent_hash, paragraph_id_header, metadata_header_fields);
    }

  return 0;
}
