#ifndef WC_CACHE_H_
#define WC_CACHE_H_

#include <opencv2/core/core.hpp>

#include <vector>

#include "types.h"


namespace wc {


class Cache {
public:
  // Static class.
  Cache() = delete;


  // Load the matrix in `dir` under the name `leaf`. If the matrix does not
  // exist, then an empty matrix is returned instead. In this case, the `data`
  // member of the returned matrix is null.
  static cv::Mat load_mat(const std::string& dir, const std::string& leaf);


  // Store `mat` in `dir` under the name `leaf`.
  static void store_mat(const std::string& dir, const std::string& leaf, 
      const cv::Mat& mat);

 
  // Return a mapping from webcam identifiers to representative descriptors.
  static DescriptorsMap load_descriptors(const std::vector<std::string>& ids);


  // Return a mapping from webcam identifiers to features.
  static FeaturesMap load_features(const std::vector<std::string>& ids);


  // Return the vocabulary matrix. Each row is a word.
  static cv::Mat load_vocabulary();


  // Directories that certain matrices are stored in.
  static const std::string kDescriptorsDir;
  static const std::string kFeaturesDir;
  static const std::string kVocabularyDir;
  static const std::string kClustersDir;
  static const std::string kClustersVisDir;
};


}  // namspace wc


#endif  // WC_CACHE_H_
