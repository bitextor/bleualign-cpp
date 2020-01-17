
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

    std::string PieceToString(StringPiece sp) {
      return std::string(sp.data(), static_cast<std::string::size_type>(sp.size()));
    }

    std::string b64decode(const StringPiece &data) {
        using namespace boost::archive::iterators;
        using It = transform_width<binary_from_base64<const char*>, 8, 6>;
        return boost::algorithm::trim_right_copy_if(std::string(It(data.data()), It(data.data() + data.length())), [](char c) {
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
      if (it != last_it) vec.push_back(std::string(last_it, it));
    }

    void SplitStringPiece(std::vector<StringPiece> &vec, StringPiece sp, char c, size_t pos, size_t max_split) {
      vec.clear();

      if (sp.size() <= 0 || pos >= unsigned(sp.size()))
        return;

      size_t num_split = 0;
      size_t res = std::find(sp.data() + pos, sp.data() + sp.length(), c) - sp.data();
      while (res < unsigned(sp.data() + sp.length() - sp.data())) {
        if (max_split and num_split >= max_split) {
          res = sp.length();
          break;
        }

        vec.push_back(sp.substr(pos, res - pos));

        pos = res + 1;
        res = std::find(sp.data() + pos, sp.data() + sp.length(), c) - sp.data();
        num_split += 1;
      }

      vec.push_back(sp.substr(pos, res - pos));

    }

    void SplitStringPiece(std::vector<StringPiece> &vec, StringPiece sp, int (*check_func)(int), size_t pos,
                          size_t max_split) {
      vec.clear();

      if (sp.size() <= 0 || pos >= unsigned(sp.size()))
        return;

      size_t num_split = 0;
      size_t res = std::find_if(sp.data() + pos, sp.data() + sp.length(), boost::ptr_fun(isspace)) - sp.data();
      while (res < unsigned(sp.data() + sp.length() - sp.data())) {
        if (max_split and num_split >= max_split) {
          res = sp.length();
          break;
        }

        vec.push_back(sp.substr(pos, res - pos));

        pos = res + 1;
        res = std::find_if(sp.data() + pos, sp.data() + sp.length(), boost::ptr_fun(*check_func)) - sp.data();
        num_split += 1;
      }

      vec.push_back(sp.substr(pos, res - pos));

    }


} // namespace utils
