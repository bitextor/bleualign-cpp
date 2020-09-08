
#include "common.h"

#include <iostream>
#include <vector>

#include <boost/functional.hpp>
#include <boost/algorithm/string.hpp>

namespace utils {
    void SplitString(std::vector<std::string> &vec, const std::string& str, char delimiter, bool trim){
      vec.clear();
      if (str.empty()) return;
      std::string::const_iterator last_it = str.begin();
      std::string::const_iterator it = std::find(last_it, str.end(), delimiter);
      while (it != str.end()){
        vec.emplace_back(std::string(last_it, it));
        last_it = it + 1;
        it = std::find(last_it, str.end(), delimiter);
      }
      if (!trim || last_it != it)
        vec.emplace_back(std::string(last_it, it));
    }

    void DecodeAndSplit(std::vector<std::string> &vec, const std::string &str, char delimiter, bool trim){
        std::string decoded = std::string(binary_text(str.begin()), binary_text(str.end()));
        boost::trim_right_if(decoded, [](char c) {
            return c == '\0';
        });
        SplitString(vec, decoded, delimiter, trim);
    }
} // namespace utils
