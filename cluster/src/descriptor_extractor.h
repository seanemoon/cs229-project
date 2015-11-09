#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>


namespace wc {


class DescriptorExtractor {
public:
  // Creates a DescriptorExtractor which uses the algorithm specified by
  // `detector_type` to detect keypoints and the descriptor type specified by
  // `descriptor_type` to extract descriptors from the keypoints.  This class
  // supports the arguments supported by OpenCV. The the time of writing, the
  // following were supported. Please refer to OpenCV documentation for the
  // ground truth of currently supported options.
  //
  // `detector_type`
  //   "FAST"        FastFeatureDetector
  //   "STAR"        StarFeatureDetector
  //   "SIFT"        SIFT (nonfree module)
  //   "SURF"        SURF (nonfree module)
  //   "ORB"         ORB
  //   "BRISK"       BRISK
  //   "MSER"        MSER
  //   "GFTT"        GoodFeaturesToTrackDetector
  //   "HARRIS"      GoodFeaturesToTrackDetector with Harris detector enabled
  //   "Dense"       DenseFeatureDetector
  //   "SimpleBlob"  SimpleBlobDetector
  // `descriptor_type`
  //   "SIFT"        SIFT
  //   "SURF"        SURF
  //   "BRIEF"       BriefDescriptorExtractor
  //   "BRISK"       BRISK
  //   "ORB"         ORB
  //   "FREAK"       FREAK
  DescriptorExtractor(const std::string& descriptor_type,
      const std::string& detector_type)
  : extractor_{cv::DescriptorExtractor::create(descriptor_type)},
    detector_{cv::FeatureDetector::create(detector_type)} {}

  // Extract descriptors from the given image(s).
  cv::Mat extract(const cv::Mat& image) const;
  std::vector<cv::Mat> extract(const std::vector<cv::Mat>& images) const;


private:
  cv::Ptr<cv::DescriptorExtractor> extractor_;
  cv::Ptr<cv::FeatureDetector> detector_;
};


}  // namespace wc
