#include <descriptor_extractor.h>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "random_selector.h"


namespace wc {


void DescriptorExtractor::SortKeypoints(std::vector<cv::KeyPoint>& keypoints) {
  std::sort(keypoints.begin(), keypoints.end(), 
      [](const cv::KeyPoint &a, const cv::KeyPoint &b) -> bool {
        return a.response < b.response;
  });
}


cv::Mat DescriptorExtractor::extract(const cv::Mat& image,
    size_t max_descriptors) const {
  std::vector<cv::KeyPoint> keypoints;
  cv::Mat descriptors;

  detector_->detect(image, keypoints);

  if (max_descriptors) {
    SortKeypoints(keypoints);
    keypoints.resize(std::min(max_descriptors, keypoints.size()));
  }

  extractor_->compute(image, keypoints, descriptors);

  return descriptors;
}


std::vector<cv::Mat> DescriptorExtractor::extract(
    const std::vector<cv::Mat>& images, size_t max_descriptors) const {
  std::vector<std::vector<cv::KeyPoint>> all_keypoints;
  std::vector<cv::Mat> all_descriptors;

  detector_->detect(images, all_keypoints);

  if (max_descriptors) {
    for (auto& keypoints : all_keypoints) {
      SortKeypoints(keypoints);
      keypoints.resize(std::min(max_descriptors, keypoints.size()));
    }
  }

  extractor_->compute(images, all_keypoints, all_descriptors);

  return all_descriptors;
}


void DescriptorExtractor::visualize_keypoints(
    const std::vector<cv::Mat>& images, const std::string& out_dir,
        size_t max_keypoints) const {
  std::vector<std::vector<cv::KeyPoint>> all_keypoints;

  detector_->detect(images, all_keypoints);

  if (max_keypoints) {
    for (auto& keypoints : all_keypoints) {
      SortKeypoints(keypoints);
      keypoints.resize(std::min(max_keypoints, keypoints.size()));
    }
  }

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
    const std::vector<cv::Mat>& frames) const {
  // Select a uniform random subset of frames to compute descriptors from.
  
  if (frames.size() < kNumFramesPerWebcam) {
    return cv::Mat{};
  }

  RandomSelector random_selector{};
  std::vector<cv::Mat> uniform_random_subset{};
  uniform_random_subset.reserve(kNumFramesPerWebcam);
  for (size_t i{0}; i < kNumFramesPerWebcam; ++i) {
    uniform_random_subset.push_back(random_selector(frames));
  }

  // Extract the descriptors from the subset of frames.
  std::cout << "extract descriptors" << std::endl;
  std::vector<cv::Mat> descriptors{extract(uniform_random_subset,
      kMaxDescriptorsPerFrame)};

  // Compute the median number of descriptors found. This will be the the
  // number of representative descriptors we will return.
  std::cout << "compute statistics" << std::endl;
  std::vector<size_t> descriptor_counts{};
  for (const auto& descriptor : descriptors) {
    descriptor_counts.push_back(descriptor.size().height);
  }
  size_t total_descriptor_count{std::accumulate(descriptor_counts.begin(),
      descriptor_counts.end(), static_cast<size_t>(0))};
  sort(descriptor_counts.begin(), descriptor_counts.end());
  size_t median_descriptor_count{
      descriptor_counts[descriptor_counts.size() / 2]};

  // Combine the descriptors into a single matrix.
  cv::Mat all_descriptors{};
  all_descriptors.reserve(total_descriptor_count);
  for (const auto& descriptor : descriptors) {
    all_descriptors.push_back(descriptor);
  }

  /*
  // Run k-means to get a representative set of descriptors.
  cv::Mat centroids;
  cv::Mat best_labels;
  const size_t num_centroids{median_descriptor_count};
  const int num_attempts{20};  // TODO(seanraff): scale with number of descriptors?
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};

  if (!num_centroids) {
    return centroids;
  }

  std::cout << "start kmeans" << std::endl;
  double compactness{cv::kmeans(all_descriptors, num_centroids, best_labels,
      term_criteria, num_attempts, flags, centroids)};

  std::cout << "Representative compactness: " 
      << compactness / total_descriptor_count << std::endl;

  for (int i {0}; i < centroids.rows; ++i) {
    normalize(descriptors.row[i], descriptors.row[i]);
  }

  // Return the centroids as the representative set of descriptors.
  return centroids;
  */

  return all_descriptors;
}


}  // namespace wc
