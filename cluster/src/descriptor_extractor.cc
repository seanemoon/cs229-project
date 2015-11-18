#include <descriptor_extractor.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "random_selector.h"


namespace wc {


void DescriptorExtractor::SortKeypoints(std::vector<cv::KeyPoint>& keypoints) {
  // Sort the keypoints by their responses.
  std::sort(keypoints.begin(), keypoints.end(), 
      [](const cv::KeyPoint &a, const cv::KeyPoint &b) -> bool {
        return a.response < b.response;
  });
}


cv::Mat DescriptorExtractor::extract(const cv::Mat& image,
    size_t max_descriptors) const {

  // Detect a set of keypoints.
  std::vector<cv::KeyPoint> keypoints;
  detector_->detect(image, keypoints);

  // If there is a limit on the number of descriptors to use, then keep the
  // keypoints with the highest response.
  cv::Mat descriptors;
  if (max_descriptors) {
    SortKeypoints(keypoints);
    keypoints.resize(std::min(max_descriptors, keypoints.size()));
  }

  // Extract descriptors for the keypoints.
  extractor_->compute(image, keypoints, descriptors);

  return descriptors;
}


std::vector<cv::Mat> DescriptorExtractor::extract(
    const std::vector<cv::Mat>& images, size_t max_descriptors) const {
  // Detect a set of keypoints.
  std::vector<std::vector<cv::KeyPoint>> all_keypoints;
  detector_->detect(images, all_keypoints);

  // If there is a limit on the number of descriptors to use, then keep the
  // keypoints with the highest response.
  std::vector<cv::Mat> all_descriptors;
  if (max_descriptors) {
    for (auto& keypoints : all_keypoints) {
      SortKeypoints(keypoints);
      keypoints.resize(std::min(max_descriptors, keypoints.size()));
    }
  }

  // Extract descriptors for the keypoints.
  extractor_->compute(images, all_keypoints, all_descriptors);

  return all_descriptors;
}


void DescriptorExtractor::visualize_keypoints(
    const std::vector<cv::Mat>& images, const std::string& out_dir,
        size_t max_keypoints) const {
  // Detect a set of keypoints.
  std::vector<std::vector<cv::KeyPoint>> all_keypoints;
  detector_->detect(images, all_keypoints);

  // If there is a limit on the number of descriptors to use, then keep the
  // keypoints with the highest response.
  if (max_keypoints) {
    for (auto& keypoints : all_keypoints) {
      SortKeypoints(keypoints);
      keypoints.resize(std::min(max_keypoints, keypoints.size()));
    }
  }

  // Draw the descriptors onto the image and save it to the given directory.
  cv::Mat out_image;
  for (size_t i{0}; i < images.size(); ++i) {
    cv::drawKeypoints(images[i], all_keypoints[i], out_image, cv::Scalar::all(-1),
        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    std::ostringstream ss;
    ss << std::setw(8) << std::setfill('0') << i;
    cv::imwrite(out_dir + "/" + ss.str() + ".jpg", out_image);
  }
}

cv::Mat DescriptorExtractor::extract_representative(
    const std::vector<cv::Mat>& frames, int num_frames_per_webcam,
        int max_descriptors_per_frame) const {
  // Degenerate webcam.
  if (frames.size() < num_frames_per_webcam) {
    return cv::Mat{};
  }

  // Sample a uniform random subset of frames from this webcam.
  RandomSelector random_selector{};
  std::vector<cv::Mat> uniform_random_subset{};
  uniform_random_subset.reserve(num_frames_per_webcam);
  for (size_t i{0}; i < num_frames_per_webcam; ++i) {
    uniform_random_subset.push_back(random_selector(frames));
  }

  // Extract the descriptors from the subset of frames.
  std::vector<cv::Mat> descriptors{extract(uniform_random_subset,
      max_descriptors_per_frame)};

  // Combine the descriptors into a single matrix.
  cv::Mat all_descriptors{};
  for (const auto& descriptor : descriptors) {
    all_descriptors.push_back(descriptor);
  }

  return all_descriptors;
}


}  // namespace wc
