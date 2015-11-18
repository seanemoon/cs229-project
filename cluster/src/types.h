#ifndef WC_TYPES_H_
#define WC_TYPES_H_

// Provides more readable type definitions.


#include <unordered_map>


namespace wc {


// Maps webcam identifiers to representative descriptors.
using DescriptorsMap = std::unordered_map<std::string, cv::Mat>;


// Maps webcam identifiers to features.
using FeaturesMap = std::unordered_map<std::string, cv::Mat>;


// Maps webcam identifiers to cluster assignments. The cluster assignment
// matrix is a thin wrapper around an integer (it lives in R^{1 x 1}).
using ClustersMap = std::unordered_map<std::string, cv::Mat>;


}  // namespace  wc


#endif  // WC_TYPES_H_
