#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "frames_manager.h"
#include "descriptor_extractor.h"
#include "cache.h"
#include "types.h"


using namespace wc;


// HYPERPARMATERS.
// Keypoint detector and descriptor extractor.
const static char* kKeypointDetector = "Dense";
const static char* kKeypointDescriptor = "SIFT";

// Clustering parameters for generating the vocabulary.
const static size_t kVocabSize{200};
const static size_t kNumVocabAttempts{20};

// Clustering parameters for generating the webcam categories.
const static int kNumClusters{100};
const static size_t kNumClusterAttempts{20};

// Sampling parameters for extracting a representative set of descriptors from
// a webcam.
const static size_t kNumFramesPerWebcam{10};
const static size_t kMaxDescriptorsPerFrame{100};



// THREADING
// We may desire a varying number of threads depending on how OpenCV implements
// library calls. For instance, k-means is implemented with multiple threads
// under OpenCV. Therefore, we will just cause congestion when trying to
// optimize for multiple cores.
// Furthermore, we may or may not want to call multithreaded code in the same
// section (extracting descriptors, computing features, etc.) as our methods
// are still highly experimental. In other words, during certain experiments we
// may want the descriptor extraction step to be multithreaded, while in other
// experiments we may find multithreading to be deleterious for descriptor
// extraction.
const static size_t kNumThreadsExtractDescriptors{12};
const static size_t kNumThreadsComputeFeatures{12};

// We currently have one global lock for all possible critical sections.
// Although this is sufficient for our current experiments, we may wish to
// change this later if we make more complex use of multithreading.
std::mutex g_mtx;

// This is the dispatch queue that the producer thread(s) (generally just one)
// use to communicate to the consumer thread(s) (generally many).
std::queue<std::string> g_queue;

// Helper function to atomically retrieve an element from the queue.
std::string atomic_pop() {
  std::string item{};
  std::lock_guard<std::mutex> guard(g_mtx);
  if (!g_queue.empty()) {
    item = g_queue.front();
    g_queue.pop();
  }
  return item;
}

// Helper function to atomically add an element to the queue.
void atomic_push(const std::string& item) {
  std::lock_guard<std::mutex> guard(g_mtx);
  g_queue.push(item);
}



// PROFILING
// Quick and dirty.
void print_current_time() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::cout << std::put_time(std::localtime(&now_c), "%c") << std::endl;
}



// MAIN HELPER FUNCTIONS
// A worker function that detects keypoints and extracts descriptors for a
// webcam. This worker persists its results using the cache. This worker runs
// as long as there are requests on the `g_queue`.
void extract_descriptors_fn(const wc::FramesManager& frames_manager) {
  const wc::DescriptorExtractor descriptor_extractor{kKeypointDetector,
      kKeypointDescriptor};
  std::string id;
  while (true) {
    id = atomic_pop();
    if (id.empty()) {
      return;
    }

    std::vector<cv::Mat> frames{frames_manager.GetFrames(id)};
    cv::Mat descriptors = descriptor_extractor.extract_representative(frames,
        kNumFramesPerWebcam, kMaxDescriptorsPerFrame);

    if (descriptors.data) {
      Cache::store_mat(Cache::kDescriptorsDir, id, descriptors);
    }
  }
}


// Helper function to extract descriptors from all of the webcams. This
// function delegates work to threads running `extract_descriptors_fn`. It
// fills the `g_queue` with webcam identifiers and waits on the worker threads
// to finish. Then, it collects the descriptors and returns the appropriate
// descriptors map.
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
  return Cache::load_descriptors(ids);
}


// A worker function that generates the vocabulary given a matrix of
// descriptors, where each row represents a descriptor. There really should
// only be one thread running the worker function, as it calls `cv::kmeans`,
// which is itself multithreaded. This function persists its results using the
// cache.
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

  Cache::store_mat(Cache::kVocabularyDir, "vocabulary", centroids);
}


// Helper function to generate the vocabulary given a map of descriptors.
// Delegates work to `generate_vocabulary_fn` after converting the map to a
// matrix where each row contains a descriptor.
cv::Mat generate_vocabulary(const DescriptorsMap& descriptors_map) {
  cv::Mat all_descriptors;
  for (const auto& pair : descriptors_map) {
    all_descriptors.push_back(pair.second);
  }
  all_descriptors.convertTo(all_descriptors, CV_32F);
  generate_vocabulary_fn(all_descriptors);

  return Cache::load_vocabulary();
}


// A worker function that computes features for a webcam. This worker persists
// its results using the cache. This worker runs as long as there are requests
// on the `g_queue`.
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

    std::vector<int> histogram(vocabulary_size);
    for (int d{0}; d < num_descriptors; ++d) {
      double smallest_distance{std::numeric_limits<double>::max()};
      size_t closest_word{0};
      for (int v{0}; v < vocabulary_size; ++v) {
        double distance{cv::norm(descriptors.row(d) - vocabulary.row(v))};
        if (distance < smallest_distance) {
          smallest_distance = distance;
          closest_word = v;
        }
      }
      histogram[closest_word] += 1;
    }

    cv::Mat features(histogram);

    if (features.data) {
      Cache::store_mat(Cache::kFeaturesDir, id, features);
    }
  }
}


// Helper function to compute the features for all of the webcams.  This
// function delegates work to threads running `compute_features_fn`. It fills
// the `g_queue` with webcam identifiers and waits on the worker threads to
// finish. Then, it collects the features and returns the appropriate features
// map.
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

  return Cache::load_features(ids);
}


// Performs the clustering on the features. This function uses OpenCV's k-means
// implementation to perform the clustering. Returns the appropriate cluster
// map.
ClustersMap cluster(const FeaturesMap& features_map, int num_clusters) {
  std::vector<std::string> ids;
  cv::Mat row;
  cv::Mat all_features;
  for (const auto& pair : features_map) {
    cv::transpose(pair.second, row);
    all_features.push_back(row);
    ids.push_back(pair.first);
  }

  cv::Mat centroids;
  cv::Mat best_labels;
  const int num_centroids{num_clusters};
  const int num_attempts{kNumClusterAttempts};
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};

  all_features.convertTo(all_features, CV_32F);
  cv::kmeans(all_features, num_centroids, best_labels, term_criteria,
      num_attempts, flags, centroids);

  ClustersMap cluster_map {};
  for (size_t i {0}; i < ids.size(); ++i) {
    cv::Mat label {};
    std::string id {ids[i]};
    label.push_back(best_labels.at<int>(i));
    Cache::store_mat(Cache::kClustersDir, id, label); 
    cluster_map[id] = label;
  }

  return cluster_map;
}


// Clusters webcams semantically based on the content of their frames.
//
// Most of the work is in extracting meaningful features. We elect to use a
// Bag-of-Featrues model, where each webcam is represented as a histogram of
// descriptors. To create this histogram, we first sample representative frames
// from each webcam (currently just a uniform random sampling) and compute
// their descriptors. We then cluster all of these descriptors into a visual
// vocabulary of fixed size. Then, given a webcam, we create a histogram of the
// closest descriptors in the visual vocabulary to descriptors in the sample of
// representative frames. These histograms are then our features that we use in
// clustering the webcams.
//
// Clustering itself is very straightforward. We just use k-means to cluster
// the webcams into a fixed number of categories.
//
// This program checkpoints its work as it proceeds. Each step is fairly time
// consuming, so checkpointing helps us speed up experiments in case we want to
// change some component. When run, this program begins at the step
// corresponding to the first empty checkpoint directory. Therefore, you can
// delete the contents of certain checkpoint directories to influence where the
// program starts. See `cache.{h, cc}` for details.
//
// In addition to producing cluster assignments, this program will also output
// a small visualization of the produced clusterings. More concretely, this
// program will create a directory for each cluster, and will then place a frame
// for each webcam assigned to that cluster within the directory.
int main(int argc, char** argv) {
  if (argc < 2 || argc > 3) {
    std::cout << "USAGE: ./COMPLETE <frames_dir> [num_clusters]" << std::endl;
    exit(2);
  }

  int num_clusters{kNumClusters};
  if (argc == 3) {
    num_clusters = std::stoi(argv[2]);
  }

  const wc::FramesManager frames_manager{argv[1]};
  std::vector<std::string> ids {frames_manager.GetWebcamIdentifiers()};

  print_current_time();
  std::cout << "Attempting to load features..." << std::endl;
  FeaturesMap features_map{Cache::load_features(ids)};
  if (features_map.empty()) {
    std::cout << "Features not found." << std::endl;

    // Extract descriptors.
    print_current_time();
    std::cout << "Attempting to load descriptors..." << std::endl;
    DescriptorsMap descriptors_map{Cache::load_descriptors(ids)};
    if (descriptors_map.empty()) {
      std::cout << "Descriptors not found." << std::endl;
      std::cout << "Extracting descriptors." << std::endl;
      descriptors_map = extract_descriptors(frames_manager);
    } else { 
      std::cout << "Using cached descriptors." << std::endl;
    }

    // Generate vocabulary.
    print_current_time();
    std::cout << "Attemping to load vocabulary..." << std::endl;
    cv::Mat vocabulary = Cache::load_vocabulary();
    if (!vocabulary.data) {
      std::cout << "Vocabulary not found." << std::endl;
      std::cout << "Generating vocabulary." << std::endl;
      vocabulary = generate_vocabulary(descriptors_map);
    } else {
      std::cout << "Using cached vocabulary." << std::endl;
    }

    // Compute features.
    std::cout << "Computing features." << std::endl;
    FeaturesMap features_map{compute_features(descriptors_map, vocabulary)};
  } else {
    std::cout << "Using cached features." << std::endl;
  }

  // Cluster the images.
  print_current_time();
  std::cout << "Clustering images." << std::endl;
  ClustersMap cluster_map{cluster(features_map, num_clusters)};

  // Visualize the clusters.
  for (const auto& pair : cluster_map) {
    cv::Mat sample = frames_manager.GetFirstFrame(pair.first);
    int label = pair.second.at<int>(0);
    if (sample.data) {
      cv::imwrite(Cache::kClustersVisDir + "/" + std::to_string(label) + "/" +
          pair.first + ".jpg", sample);
    }
  }

  return 0;
}
