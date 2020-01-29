
#ifndef FAST_BLEUALIGN_COMMON_H
#define FAST_BLEUALIGN_COMMON_H


#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>


namespace utils {


    struct match {

        struct inner {

            size_t from;
            size_t to;

            inner(size_t a, size_t b) : from(a), to(b) {};

            bool same() { return from == to; };

        };

        inner first;
        inner second;
        double score;

        match() : first(0, 0), second(0, 0) {score=0;};

        match(size_t a, size_t b, size_t c, size_t d, double s) : first(a, b), second(c, d) {score=s;};

        void print() {
          printf("(%zu, %zu) -> (%zu, %zu): %f\n", first.from, first.to, second.from, second.to, score);
        }

        bool operator==(const match &rhs) const {
          return first.from == rhs.first.from && first.to == rhs.first.to &&
                 second.from == rhs.second.from && second.to == rhs.second.to;
        }

    };


    typedef std::pair<size_t, size_t> sizet_pair;
    typedef std::vector<sizet_pair> vec_pair;
    typedef std::unordered_map<std::string, std::vector<std::string>> umap_extracted;
    typedef std::vector<std::pair<std::string, std::string>> matches_list;
    typedef std::multimap<float, std::pair<size_t, std::vector<int>>> scoremap;
    typedef std::vector<match> matches_vec;


    struct DocumentPair {
        std::string url1;
        std::string url2;
        std::vector<std::string> text1;
        std::vector<std::string> text2;
        std::vector<std::string> text1translated;
    };


    void SplitString(std::vector<std::string> &vec, const std::string &str, char c);
    std::string b64decode(const std::string &data);
} // namespace utils


#endif //FAST_BLEUALIGN_COMMON_H
