#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <memory>


// Extracts SIFT descriptors from the given image.
// Each descriptor is a normalized vector of size 128.
cv::Mat extract_sift_descriptors(const cv::Mat& image) {
  std::vector<cv::KeyPoint> keypoints;
  cv::Mat descriptors;
  cv::SiftFeatureDetector detector;
  cv::Ptr<cv::DescriptorExtractor> descriptor_extractor{
      cv::DescriptorExtractor::create("SIFT")};

  std::cout << "a" << std::endl;
  detector.detect(image, keypoints);
  std::cout << "b" << std::endl;
  if (!descriptor_extractor) {
    std::cout << "ERROR" << std::endl;
  }
  descriptor_extractor->compute(image, keypoints, descriptors);
  std::cout << "c" << std::endl;

  cv::Mat output;
  cv::drawKeypoints(image, keypoints, output);
  std::cout << keypoints.size() << std::endl;
  cv::imwrite("sift_results.jpg", output);

  return descriptors;
}

int main(int argc, char** argv) {
  const cv::Mat input = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

  if (!input.data) {
    std::cout << "No image data" << std::endl;
    return -1;
  }

  cv::Mat descriptors = extract_sift_descriptors(input);
  std::cout << descriptors.size() << std::endl;


  return 0;
}
