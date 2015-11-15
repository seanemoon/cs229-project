#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "frames_manager.h"
#include "descriptor_extractor.h"


// HYPERPARMATERS.

// The keypoint detector to use.
const static char* kKeypointDetector = "SIFT";
// The keypoint descriptor to use.
const static char* kKeypointDescriptor = "SIFT";

// Number of words in the bag-of-features representation.
const static size_t kVocabSize{300};
// Number of times to run k-means when generating the vocabulary.
const static size_t kNumVocabAttempts{20};

// Number of image categories.
const static size_t kNumClusters{3};
// Number of times to run k-means when clustering images.
const static size_t kNumClusterAttempts{3};



// Threading.
// Some keypoint detector/descriptors are implemented to be multithreaded. In
// this case, we should only use one thread.
const static size_t kNumThreadsExtractDescriptors {1};
// Computing features is done by hand, so multithreading can increase
// throughput.
const static size_t kNumThreadsComputeFeatures {12};
std::mutex g_mtx;
std::queue<std::string> g_queue;


// Threading helper functions.
std::string atomic_pop() {
  std::string item{};
  std::lock_guard<std::mutex> guard(g_mtx);
  if (!g_queue.empty()) {
    item = g_queue.front();
    g_queue.pop();
  }
  return item;
}


void atomic_push(const std::string& item) {
  std::lock_guard<std::mutex> guard(g_mtx);
  g_queue.push(item);
}


// Helpful definitions.
using DescriptorsMap = std::unordered_map<std::string, cv::Mat>;
using FeaturesMap = std::unordered_map<std::string, cv::Mat>;
using ClustersMap = std::unordered_map<std::string, cv::Mat>;


const std::string kDescriptorsDir{"descriptors"};
const std::string kFeaturesDir{"features"};
const std::string kVocabularyDir{"vocabulary"};
const std::string kClustersDir{"clusters"};


// Quick and dirty profiling.
void print_current_time() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::cout << std::put_time(std::localtime(&now_c), "%c") << std::endl;
}


// Load and store helper functions.
cv::Mat load_mat(const std::string& dir, const std::string& leaf) {
  cv::Mat mat;
  cv::FileStorage file(dir + "/" + leaf + ".yml", cv::FileStorage::READ);
  file[leaf] >> mat;
  return mat;
}


void store_mat(const std::string& dir, const std::string& leaf,
    const cv::Mat& mat) {
  cv::FileStorage file(dir + "/" + leaf + ".yml", cv::FileStorage::WRITE);
  file << leaf << mat;
}


DescriptorsMap load_descriptors(const std::vector<std::string>& ids) {
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


FeaturesMap load_features(const std::vector<std::string>& ids) {
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


cv::Mat load_vocabulary() {
  return load_mat(kVocabularyDir, "vocabulary");
}


// Main helper functions.
void extract_descriptors_fn(const wc::FramesManager& frames_manager) {
  const wc::DescriptorExtractor descriptor_extractor{kKeypointDetector,
      kKeypointDescriptor};
  std::string id;
  while (true) {
    id = atomic_pop();
    if (id.empty()) {
      return;
    }
    std::cout << id << std::endl;

    std::vector<cv::Mat> frames{frames_manager.GetFrames(id)};
    // TODO(seanrafferty): Pass hyperparams here.
    std::cout << "call extract" << std::endl;
    cv::Mat descriptors = descriptor_extractor.extract_representative(frames);
    std::cout << "return extract" << std::endl;

    if (descriptors.data) {
      std::cout << "Storing descriptors." << std::endl;
      store_mat(kDescriptorsDir, id, descriptors);
    }
  }
}


DescriptorsMap extract_descriptors(const wc::FramesManager& frames_manager) {
  std::thread threads[kNumThreadsExtractDescriptors];
  const auto ids = frames_manager.GetWebcamIdentifiers();
  for (const auto& id : ids) {
    atomic_push(id);
  }
  for (size_t i{0}; i < kNumThreadsExtractDescriptors; ++i) {
    threads[i] = std::thread(extract_descriptors_fn, frames_manager);
  }
  for (auto& thread : threads) {
    thread.join();
  }
  return load_descriptors(ids);
}


void generate_vocabulary_fn(const cv::Mat& all_descriptors) {
  cv::Mat centroids;
  cv::Mat best_labels;
  const size_t num_centroids{kVocabSize};
  const int num_attempts{};
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};

  cv::kmeans(all_descriptors, num_centroids, best_labels, term_criteria,
      num_attempts, flags, centroids);

  store_mat(kVocabularyDir, "vocabulary", centroids);
}

cv::Mat generate_vocabulary(const DescriptorsMap& descriptors_map) {
  // TODO(seanrafferty): Consider whether we are losing our ordering here. This would
  // imply that everything we learn is now random.
  cv::Mat all_descriptors;
  for (const auto& pair : descriptors_map) {
    all_descriptors.push_back(pair.second);
  }

  generate_vocabulary_fn(all_descriptors);

  return load_vocabulary();
}


void compute_features_fn(const DescriptorsMap& descriptors_map,
    const cv::Mat& vocabulary) {
  std::string id;
  while (true) {
    id = atomic_pop();
    if (id.empty()) {
      return;
    }

    const cv::Mat descriptors = descriptors_map.at(id);
    int num_descriptors = descriptors.size().height;
    int vocabulary_size = vocabulary.size().height;

    std::cout << "num descriptors: " << num_descriptors << std::endl;
    std::cout << "vocabulary size: " << vocabulary_size << std::endl;
    std::cout << "vocabulary.size(): " << vocabulary.size() << std::endl;

    std::vector<int> histogram(vocabulary_size);
    for (int d{0}; d < num_descriptors; ++d) {
      std::cout << "A" << std::endl;
      double smallest_distance{std::numeric_limits<double>::max()};
      size_t closest_word{0};
      for (int v{0}; v < vocabulary_size; ++v) {
        std::cout << "B" << std::endl;
        double distance{cv::norm(descriptors.row(d) - vocabulary.row(v))};
        if (distance < smallest_distance) {
          smallest_distance = distance;
          closest_word = v;
        }
      }
      histogram[closest_word] += 1;
    }

    std::cout << "C" << std::endl;
    cv::Mat features(histogram);

    if (features.data) {
      store_mat(kFeaturesDir, id, features);
    }
  }
}


FeaturesMap compute_features(const DescriptorsMap& descriptors_map,
    const cv::Mat& vocabulary) {
  std::thread threads[kNumThreadsComputeFeatures];
  std::vector<std::string> ids;
  for (const auto& pair : descriptors_map) {
    atomic_push(pair.first);
    ids.push_back(pair.first);
  }
  for (size_t i{0}; i < kNumThreadsComputeFeatures; ++i) {
    threads[i] = std::thread(compute_features_fn, descriptors_map, vocabulary);
  }
  for (auto& thread : threads) {
    thread.join();
  }

  return load_features(ids);
}


ClustersMap cluster(const FeaturesMap& features_map) {
  std::cout << features_map.size() << std::endl;

  std::vector<std::string> ids;
  cv::Mat all_features(128, 0, CV_32F);
  for (const auto& pair : features_map) {
    all_features.push_back(pair.second);
    ids.push_back(pair.first);
  }

  std::cout << ids.size() << std::endl;

  cv::Mat centroids;
  cv::Mat best_labels;
  const size_t num_centroids{kNumClusters};
  const int num_attempts{kNumClusterAttempts};
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};

  std::cout << all_features.size() << std::endl;
  std::cout << all_features.type() << std::endl;
  std::cout << num_centroids << std::endl;

  cv::kmeans(all_features, num_centroids, best_labels, term_criteria,
      num_attempts, flags, centroids);

  ClustersMap cluster_map {};
  for (size_t i {0}; i < ids.size(); ++i) {
    cv::Mat label {};
    std::string id {ids[i]};
    label.push_back(best_labels.at<int>(i));
    store_mat(kClustersDir, id, label); 
    cluster_map[id] = label;
  }

  return cluster_map;
}


int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "USAGE: ./COMPLETE <frames_dir>" << std::endl;
    exit(2);
  }

  const wc::FramesManager frames_manager{argv[1]};
  std::vector<std::string> ids {frames_manager.GetWebcamIdentifiers()};

  print_current_time();
  DescriptorsMap descriptors_map{load_descriptors(ids)};
  if (descriptors_map.empty()) {
    std::cout << "Extracting descriptors." << std::endl;
    descriptors_map = extract_descriptors(frames_manager);
  } else { 
    std::cout << "Using cached descriptors." << std::endl;
  }

  print_current_time();
  cv::Mat vocabulary = load_vocabulary();
  if (!vocabulary.data) {
    std::cout << "Generating vocabulary." << std::endl;
    vocabulary = generate_vocabulary(descriptors_map);
  } else {
    std::cout << "Using cached vocabulary." << std::endl;
  }

  print_current_time();
  FeaturesMap features_map{load_features(ids)};
  if (features_map.empty()) {
    std::cout << "Computing features." << std::endl;
    FeaturesMap features_map{compute_features(descriptors_map, vocabulary)};
  } else {
    std::cout << "Using cached features." << std::endl;
  }

  print_current_time();
  std::cout << "Clustering images." << std::endl;
  ClustersMap cluster_map{cluster(features_map)};

  return 0;
}
