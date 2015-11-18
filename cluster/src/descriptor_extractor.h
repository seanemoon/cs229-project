#ifndef WC_DESCRIPTOR_EXTRACTOR_H_
#define WC_DESCRIPTOR_EXTRACTOR_H_

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
  DescriptorExtractor(const std::string& detector_type,
      const std::string& descriptor_type)
  : detector_{cv::FeatureDetector::create(detector_type)},
    extractor_{cv::DescriptorExtractor::create(descriptor_type)} {}


  // Extract descriptors from the given image(s). If `max_descriptors` is
  // specified, then no more than `max_descriptors` will be returned for each
  // image.  Furthermore, these descriptors are guaranteed to originate from
  // the keypoints with the highest responses.
  
  // Extracts descriptors for one image.
  cv::Mat extract(const cv::Mat& image, size_t max_descriptors = 0) const;

  // Extracts descriptors for multiple images.
  std::vector<cv::Mat> extract(const std::vector<cv::Mat>& images,
      size_t max_descriptors = 0) const;


  // First detects keypoints and extracts descriptors for the given images.
  // Then, overlays the descriptors onto the corresponding image. Finally, the
  // modified images are persisted to the specified directory. If desired, you
  // may specify the maximum number of keypoints you would like to draw for
  // each image.
  void visualize_keypoints(const std::vector<cv::Mat>& images,
      const std::string& out_dir, size_t max_keypoints = 0) const;

  cv::Mat extract_representative(const std::vector<cv::Mat>& frames,
      int num_frames_per_webcam, int max_descriptors_per_frame) const;



private:
  static void SortKeypoints(std::vector<cv::KeyPoint>& keypoints);

  cv::Ptr<cv::FeatureDetector> detector_;
  cv::Ptr<cv::DescriptorExtractor> extractor_;
};


}  // namespace wc

#endif  // WC_DESCRIPTOR_EXTRACTOR_H_
