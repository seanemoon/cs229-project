#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <unordered_map>

#include "frames_manager.h"
#include "descriptor_extractor.h"


const static size_t kNumThreads {12};

std::mutex g_requests_mutex;
std::mutex g_filesys_mutex;
std::queue<std::string> g_requests;

void extract_descriptors(const std::string& input_dir,
    const std::string output_dir) {
  const wc::FramesManager manager{input_dir};
  const wc::DescriptorExtractor extractor{"SURF", "SURF"};
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

    std::vector<cv::Mat> frames{manager.GetFrames(identifier)};

    cv::Mat descriptors = extractor.extract_representative(frames);

    // Store the results.
    if (descriptors.data) {
      std::lock_guard<std::mutex> guard(g_filesys_mutex);
      cv::FileStorage file(output_dir + "/" + identifier + ".yml", cv::FileStorage::WRITE);
      file << "descriptors" << descriptors;
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
  if (!g_requests.empty()) {
    std::thread threads[kNumThreads];
    for (size_t i{0}; i < kNumThreads; ++i) {
      threads[i] = std::thread(extract_descriptors, argv[1], argv[2]);
    }
    for (std::thread& thread : threads) {
      thread.join();
    }
  }

  return 0;
}
