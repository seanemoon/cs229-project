#include <frames_manager.h>

#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <vector>
#include <map>


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
  std::map<std::string, cv::Mat> frames_map{};

  for (directory_iterator itr{webcam_path}, end_itr{}; itr != end_itr; ++itr) {
    cv::Mat frame{cv::imread(itr->path().string(), CV_LOAD_IMAGE_GRAYSCALE)};
    if (frame.data) {
      frames_map[itr->path().leaf().string()] = frame;
    }
  }

  std::vector<cv::Mat> frames{};
  for (const auto& entry : frames_map) {
    frames.push_back(entry.second);
  }
  return frames;
}


cv::Mat FramesManager::GetFirstFrame(
    const std::string& webcam_identifier) const {
  using boost::filesystem::directory_iterator;
  using boost::filesystem::path;

  const path webcam_path = directory_/webcam_identifier;
  std::map<std::string, cv::Mat> frames_map{};

  for (directory_iterator itr{webcam_path}, end_itr{}; itr != end_itr; ++itr) {
    cv::Mat frame{cv::imread(itr->path().string(), CV_LOAD_IMAGE_COLOR)};
    return frame;
  }
  
  return cv::Mat{};
}


}  // namespace wc
