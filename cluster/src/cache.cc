#include "cache.h"

#include <opencv2/core/core.hpp>

#include "types.h"

namespace wc {

const std::string Cache::kDescriptorsDir{"descriptors"};
const std::string Cache::kFeaturesDir{"features"};
const std::string Cache::kVocabularyDir{"vocabulary"};
const std::string Cache::kClustersDir{"clusters"};
const std::string Cache::kClustersVisDir{"clusters_vis"};

cv::Mat Cache::load_mat(const std::string& dir, const std::string& leaf) {
  cv::Mat mat;
  cv::FileStorage file(dir + "/" + leaf + ".yml", cv::FileStorage::READ);
  file[leaf] >> mat;
  return mat;
}

void Cache::store_mat(const std::string& dir, const std::string& leaf,
    const cv::Mat& mat) {
  cv::FileStorage file(dir + "/" + leaf + ".yml", cv::FileStorage::WRITE);
  file << leaf << mat;
}

DescriptorsMap Cache::load_descriptors(const std::vector<std::string>& ids) {
  DescriptorsMap map;
  cv::Mat mat;
  for (const auto& id : ids) {
    mat = load_mat(kDescriptorsDir, id);
    if (mat.data) {
      map[id] = mat;
    }
  }
  return map;
}

FeaturesMap Cache::load_features(const std::vector<std::string>& ids) {
  FeaturesMap map;
  cv::Mat mat;
  for (const auto& id : ids) {
    mat = load_mat(kFeaturesDir, id);
    if (mat.data) {
      map[id] = mat;
    }
  }
  return map;
}

cv::Mat Cache::load_vocabulary() {
  return load_mat(kVocabularyDir, "vocabulary");
}


}  // namspace wc
