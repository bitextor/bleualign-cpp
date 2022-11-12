# Bleualign-cpp
C++ sentence alignment tool based on [Bleualign](https://github.com/rsennrich/Bleualign).
Bleualign-cpp is expected to be used together with [document-aligner](https://github.com/bitextor/bitextor/tree/master/document-aligner).

### Requirements
- GCC, C++11 compiler
- [Boost](https://www.boost.org/) 1.58.0 or later
- [CMake](https://cmake.org/download/) 3.7.2 or later
- [GTest](https://github.com/google/googletest) (for tests)
- [kpu/preprocess](https://github.com/kpu/preprocess) (already included in this repository as a submodule)


### Compile with CMake

```bash
mkdir build
cd build
cmake .. -DBUILD_TEST=on -DCMAKE_BUILD_TYPE=Release
# use `cmake .. -DBUILD_TEST=on -DCMAKE_BUILD_TYPE=Release -DPREPROCESS_PATH=/home/user/preprocess/` if you use other 'preprocess' folder
make -j 4
tests/test_all
```


### Usage

Bleualign-cpp takes two texts in two different languages and aligns them to produce parallel sentences. To this end, it also needs a translation of one of these texts.

Input format is `url1 <tab> url2 <tab> text1 <tab> text2 <tab> text1translated [ <tab> text2processed ] [ <tab> text1metadata <tab> text2metadata ]` per line. Every text column is encoded as base64. After decoding text columns, they should contain a single sentence per line. The translation (`text1translated`) should correspond line-by-line with the original text (`text1`). The first line will be a header where, following the explained format, the expected fields are: `src_url <tab> trg_url <tab> src_text <tab> trg_text <tab> src_translated [ <tab> trg_translated ] [ <tab> src_metadata <tab> trg_metadata ]`. The first line of the output will contain a header as well which, depending on the provided arguments, the fields will be: `src_url <tab> trg_url <tab> src_text <tab> trg_text <tab> bleualign_score [ <tab> src_deferred_hash <tab> trg_deferred_hash ] [ <tab> src_metadata_field_1 <tab> trg_metadata_field_1 ... ]`.

Optionally a processed version of `text2` can be provided, as a sixth column, that better matches the processing applied to `text1translated` to help with calculating alignment scores. The output of bleualign will only mention `text1` and `text2`.

Bleualign-cpp outputs aligned sentences to standard output. Output format is (mandatory fields only): `url1 <tab> url2 <tab> source_sentence <tab> target_sentence <tab> score` per line.

Bleualign receives input by stdin and writes output to stdout.

##### Optional Parameters
* **--help** - Print help dialog
* **--bleu_threshold** - Sentence-level BLEU score threshold (Default: 0.0)
* **--print-sent-hash** - Print hash for each sentence
* **--metadata-header-fields** - Language agnostic comma separated list of metadata header fields (prefix `src_` and `trg_` will be added after)
