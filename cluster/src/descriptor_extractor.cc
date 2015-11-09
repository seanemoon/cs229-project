#include <descriptor_extractor.h>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>


namespace wc {


cv::Mat DescriptorExtractor::extract(const cv::Mat& image) const {
  std::vector<cv::KeyPoint> keypoints;
  cv::Mat descriptors;

  detector_->detect(image, keypoints);
  extractor_->compute(image, keypoints, descriptors);

  return descriptors;
}

std::vector<cv::Mat> DescriptorExtractor::extract(
    const std::vector<cv::Mat>& images) const {
  std::vector<std::vector<cv::KeyPoint>> keypoints;
  std::vector<cv::Mat> descriptors;

  detector_->detect(images, keypoints);
  extractor_->compute(images, keypoints, descriptors);

  return descriptors;
}


}  // namespace wc
