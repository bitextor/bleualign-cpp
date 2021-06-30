
#include "gtest/gtest.h"
#include "../src/utils/common.h"

#include <boost/functional.hpp>


namespace {

    TEST(utils, test_common_SplitString) {

      std::string s1("This is a text with many single spaces and   a  few     gaps   . ");

      std::vector<std::string> s_vec1;
      utils::SplitString(s_vec1, s1, ' ');
      ASSERT_EQ(s_vec1.size(), 23);
      std::vector<std::string> expected_vec1 = {"This", "is", "a", "text", "with", "many", "single", "spaces", "and",
                                                "", "", "a", "", "few", "", "", "", "", "gaps", "", "", ".", ""};
      for (size_t i = 0; i < expected_vec1.size(); ++i) {
        if (i >= s_vec1.size()) FAIL();
        ASSERT_EQ(expected_vec1.at(i), s_vec1.at(i));
      }
    }

    TEST(utils, test_common_DecodeAndSplit) {

      std::string s1("VGhpcyBpcyBhIHRleHQgd2l0aCBtYW55IHNpbmdsZSBzcGFjZXMgYW5kICAgYSAgZmV3ICAgICBnYXBzICAgLiA=");
      std::vector<std::string> s_vec1;
      utils::DecodeAndSplit(s_vec1, s1, ' ');
      std::vector<std::string> expected_vec1 = {"This", "is", "a", "text", "with", "many", "single", "spaces", "and",
                                                  "", "", "a", "", "few", "", "", "", "", "gaps", "", "", ".", ""};
      ASSERT_EQ(s_vec1.size(), expected_vec1.size());
      for (size_t i=0; i<expected_vec1.size(); ++i){
        ASSERT_EQ(expected_vec1.at(i), s_vec1.at(i));
      }

    }
} // namespace
