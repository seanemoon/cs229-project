#include <frames_manager.h>

#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


namespace wc {


std::vector<std::string> FramesManager::GetWebcamIdentifiers() const {
  using boost::filesystem::directory_iterator;
  using boost::filesystem::is_directory;

  directory_iterator end_itr;
  std::vector<std::string> identifiers;

  for (directory_iterator itr{directory_}, end_itr; itr != end_itr; ++itr) {
    if (is_directory(itr->status())) {
      identifiers.push_back(itr->path().leaf().string());
    }
  }

  return identifiers;
}


std::vector<cv::Mat> FramesManager::GetFrames(
    const std::string& webcam_identifier) const {
  using boost::filesystem::directory_iterator;
  using boost::filesystem::path;

  const path webcam_path = directory_/webcam_identifier;
  std::vector<cv::Mat> frames{};
  cv::Mat frame{};

  for (directory_iterator itr{webcam_path}, end_itr{}; itr != end_itr; ++itr) {
    frame = cv::imread(itr->path().string(), CV_LOAD_IMAGE_GRAYSCALE);
    if (frame.data) {
      frames.push_back(frame);
    }
  }

  return frames;
}


}  // namespace wc
