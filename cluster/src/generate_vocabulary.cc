#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <mutex>
#include <thread>
#include <queue>

#include "frames_manager.h"

const static size_t kNumThreads {12};
const static size_t kVocabSize {1000};

std::mutex g_all_descriptors_mutex;
std::mutex g_requests_mutex;
std::mutex g_filesys_mutex;

std::queue<std::string> g_requests;
cv::Mat g_all_descriptors;

void extract_descriptors(const std::string& descriptors_dir) {
  while (true) {
    // Get the next request.
    std::string identifier;
    {
      std::lock_guard<std::mutex> guard(g_requests_mutex);
      if (g_requests.empty()) {
        return;
      } else {
        identifier = g_requests.front();
        g_requests.pop();
      }
    }

    // Load the descriptors from the filesystem.
    cv::Mat descriptors;
    {
      std::lock_guard<std::mutex> guard(g_filesys_mutex);
      cv::FileStorage file(descriptors_dir + "/" + identifier + ".yml", cv::FileStorage::READ);
      file["descriptors"] >> descriptors;
    }

    // Save the results in memory.
    if (descriptors.data) {
      std::lock_guard<std::mutex> guard(g_all_descriptors_mutex);
      g_all_descriptors.push_back(descriptors);
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    exit(2);
  }

  const wc::FramesManager manager{argv[1]};
  for (const std::string& identifier : manager.GetWebcamIdentifiers()) {
    g_requests.push(identifier);
  }

  std::cout << "Loading descriptors." << std::endl;
  if (!g_requests.empty()) {
    std::thread threads[kNumThreads];
    for (size_t i{0}; i < kNumThreads; ++i) {
      threads[i] = std::thread(extract_descriptors, argv[2]);
    }
    for (std::thread& thread : threads) {
      thread.join();
    }
  }

  std::cout << "Running k-means." << std::endl;

  cv::Mat centroids;
  cv::Mat best_labels;
  const size_t num_centroids{kVocabSize};
  const int num_attempts{50};  // TODO(seanraff): scale with number of descriptors?
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};
  double compactness{cv::kmeans(g_all_descriptors, num_centroids, best_labels,
      term_criteria, num_attempts, flags, centroids)};

  std::cout << "Compactness: " << compactness << std::endl;

  cv::FileStorage file("vocabulary.yml", cv::FileStorage::WRITE);
  file << "vocabulary" << centroids;

  return 0;
}
