
#include "common.h"

#include <iostream>
#include <vector>
#include <regex>
#include <ctype.h>

#include <boost/functional.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace utils {

    std::string b64decode(const std::string &data) {
        using namespace boost::archive::iterators;
        using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
        return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(data)), It(std::end(data))), [](char c) {
            return c == '\0';
        });
    }

    void SplitString(std::vector<std::string> &vec, const std::string& str, char c){
      vec.clear();
      if (str.size() <= 0) return;
      std::string::const_iterator last_it = str.begin();
      std::string::const_iterator it = std::find(last_it, str.end(), c);
      while (it != str.end()){
        vec.push_back(std::string(last_it, it));
        last_it = it + 1;
        it = std::find(last_it, str.end(), c);
      }
      vec.push_back(std::string(last_it, it));
    }

} // namespace utils
