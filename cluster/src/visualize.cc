#include "frames_manager.h"
#include "descriptor_extractor.h"

#include <iostream>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "USAGE: VISUALIZE frames_path" << std::endl;
  }
  const wc::FramesManager manager{argv[1]};
  const std::string webcam{"opentopia_00012342"};

  const wc::DescriptorExtractor extractor{"SURF", "SURF"};
  std::vector<cv::Mat> frames{manager.GetFrames(webcam)};
  extractor.visualize_keypoints(frames, "visualized", 200);
}
