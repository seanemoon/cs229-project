#ifndef WC_FRAMES_MANAGER_H_
#define WC_FRAMES_MANAGER_H_

#include <opencv2/core/core.hpp>
#include <boost/filesystem.hpp>


namespace wc {


class FramesManager {
public:
  // Construct a FramesManager which manages the frames in `directory`.
  FramesManager(const std::string& directory)
    : directory_{directory} {}

  // Returns a list of webcam identifiers in an arbitrary order.
  std::vector<std::string> GetWebcamIdentifiers() const;

  // Returns a list of frames for the webcam specified by `webcam_identifier`.
  // The returned list is in ascending lexicorigraphical order with respect to
  // the filenames of the frames.
  std::vector<cv::Mat> GetFrames(const std::string& webcam_identifier) const;

  // Returns the first frame for the webcam specified by `webcam_identifier`.
  cv::Mat GetFirstFrame(const std::string& webcam_identifier) const;

private:
  // Frame directory.
  const boost::filesystem::path directory_;
};


}  // namespace wc


#endif  // WC_FRAMES_MANAGER_H_
