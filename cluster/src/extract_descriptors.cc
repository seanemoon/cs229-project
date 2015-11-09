#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include "descriptor_extractor.h"
#include "frames_manager.h"

int main(int argc, char** argv) {
  const wc::FramesManager manager{argv[1]};

  std::vector<std::string> webcam_identifiers{manager.GetWebcamIdentifiers()};

  if (!webcam_identifiers.empty()) {
    wc::DescriptorExtractor descriptor_extractor{"SIFT", "SIFT"};
    std::vector<cv::Mat> frames{manager.GetFrames(webcam_identifiers[0])};
    std::cout << "Finished extracting frames." << std::endl;
    std::vector<cv::Mat> descriptors = descriptor_extractor.extract(frames);

    std::cout << "Extracted from" << descriptors.size() << " frames.";
    std::cout << std::endl;
  }

}
