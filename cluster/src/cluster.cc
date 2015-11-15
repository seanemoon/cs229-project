#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include "frames_manager.h"


// Cluster the images to get similar ones.
int main(int argc, char** argv) {
  const wc::FramesManager manager{argv[1]};
  cv::Mat all_image_features;

  std::vector<std::string> identifiers = manager.GetWebcamIdentifiers();
  for (const std::string& identifier : identifiers) {
    cv::Mat image_features;
    cv::FileStorage file(std::string(argv[2]) + "/" + identifier + ".yml", cv::FileStorage::READ);
    file["feature"] >> image_features;
    all_image_features.push_back(image_features);
  }

  all_image_features.convertTo(all_image_features, CV_32F);
  std::cout << all_image_features.dims << std::endl;
  std::cout << all_image_features.type() << std::endl;


  cv::Mat centroids;
  cv::Mat best_labels;
  const size_t num_centroids{3};
  const int num_attempts{1};  // TODO(seanraff): scale with number of descriptors?
  const double epsilon{0.1};
  const cv::TermCriteria term_criteria(cv::TermCriteria::EPS, 0, epsilon);
  const int flags{cv::KMEANS_PP_CENTERS};

  std::cout << all_image_features.size() << std::endl;
  double compactness{cv::kmeans(all_image_features, num_centroids, best_labels,
      term_criteria, num_attempts, flags, centroids)};

  for (size_t i {0}; i < identifiers.size(); ++i) {
    cv::Mat sample = manager.GetFirstFrame(identifiers[i]);
    auto id = identifiers[i];
    auto label = std::to_string(best_labels.at<int>(i));
    if (sample.data) {
      cv::imwrite(std::string(argv[3]) + "/" + label  + "/"  + id + ".jpg", sample);
    }
  }
}
