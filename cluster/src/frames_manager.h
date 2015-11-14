#ifndef WC_FRAMES_MANAGER_H_
#define WC_FRAMES_MANAGER_H_

#include <opencv2/core/core.hpp>
#include <boost/filesystem.hpp>

namespace wc {


class FramesManager {
public:
  FramesManager(const std::string& directory)
    : directory_{directory} {}

  std::vector<std::string> GetWebcamIdentifiers() const;

  std::vector<cv::Mat> GetFrames(const std::string& webcam_identifier) const;

private:
  const boost::filesystem::path directory_;
};


}  // namespace wc


#endif  // WC_FRAMES_MANAGER_H_
