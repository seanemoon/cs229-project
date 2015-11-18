#include "frames_manager.h"
#include "descriptor_extractor.h"

#include <iostream>


// The maximum number of descriptors to visualize.
const static int kMaxDescriptorsToShow {200};


// A simple program to visualize the types of keypoints and descriptors
// found by given configurations of the DescriptorExtractor. Currently,
// the results are hard-coded to be output to the `visualized` directory
// in the current working directory.
int main(int argc, char** argv) {
  if (argc < 2 || argc > 5) {
    std::cout << "USAGE: VISUALIZE <frames_path> [webcam_id] [detector] [extractor]" << std::endl;
  }

  std::string detector{"SIFT"};
  std::string descriptor{"SIFT"};
  std::string webcam_id{"opentopia_00012342"};

  if (argc > 2) {
    webcam_id  = std::string{argv[2]};
  }
  if (argc > 3) {
    detector = std::string{argv[3]};
  }
  if (argc > 4) {
    descriptor = std::string{argv[4]};
  }

  const wc::FramesManager manager{argv[1]};

  const wc::DescriptorExtractor extractor{detector, descriptor};
  std::vector<cv::Mat> frames{manager.GetFrames(webcam_id)};
  extractor.visualize_keypoints(frames, "visualized", kMaxDescriptorsToShow);
}
