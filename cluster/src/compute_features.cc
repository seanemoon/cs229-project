#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <queue>

#include "frames_manager.h"
#include "descriptor_extractor.h"


const static size_t kNumThreads {1};

std::mutex g_requests_mutex;
std::mutex g_filesys_mutex;
std::queue<std::string> g_requests;

void compute_features(const std::string& descriptors_dir,
    const std::string& vocabulary_file,
    const std::string& output_dir) {
  std::string identifier;
  while (true) {
    // Get the next request.
    {
      std::lock_guard<std::mutex> guard(g_requests_mutex);
      if (g_requests.empty()) {
        return;
      } else {
        identifier = g_requests.front();
        g_requests.pop();
      }
    }

    // Load the descriptors and vocabulary.
    cv::Mat vocabulary;
    cv::Mat descriptors;
    {
      cv::FileStorage file(descriptors_dir + "/" + identifier + ".yml", cv::FileStorage::READ);
      std::lock_guard<std::mutex> guard(g_filesys_mutex);
      file["descriptors"] >> descriptors;
      file.release();
      file.open(vocabulary_file, cv::FileStorage::READ);
      file["vocabulary"] >> vocabulary;
      std::cout << "Loaded data." << std::endl;
    }

    // Calculate feature histogram.
    std::vector<int> histogram(vocabulary.size().height);
    for (int r_d {0}; r_d < descriptors.size().height; ++r_d) {
      double distance{0.0};
      double smallest_distance{std::numeric_limits<double>::max()};
      size_t closest_word_idx{0};
      for (int r_v {0}; r_v < vocabulary.size().height; ++r_v) {
        distance = cv::norm(descriptors.row(r_d) - vocabulary.row(r_v));
        if (distance < smallest_distance) {
          smallest_distance = distance;
          closest_word_idx = r_v;
        }
      }
      histogram[closest_word_idx] += 1;
    }
    cv::Mat features(histogram);

    // Store the results.
    if (descriptors.data) {
      std::lock_guard<std::mutex> guard(g_filesys_mutex);
      cv::FileStorage file(output_dir + "/" + identifier + ".yml", cv::FileStorage::WRITE);
      file << "feature" << features;
      std::cout << "Saved features." << std::endl;
    }
  }
}


int main(int argc, char** argv) {
  if (argc != 5) {
    exit(2);
  }


  const wc::FramesManager manager{argv[1]};
  for (const std::string& identifier : manager.GetWebcamIdentifiers()) {
    g_requests.push(identifier);
  }
  if (!g_requests.empty()) {
    std::thread threads[kNumThreads];
    for (size_t i{0}; i < kNumThreads; ++i) {
      threads[i] = std::thread(compute_features, argv[2], argv[3], argv[4]);
    }
    for (std::thread& thread : threads) {
      thread.join();
    }
  }

  return 0;
}
