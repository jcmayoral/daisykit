// Copyright 2021 The DaisyKit Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DAISYKIT_GRAPHS_NODES_FACE_VISUALIZER_NODE_H_
#define DAISYKIT_GRAPHS_NODES_FACE_VISUALIZER_NODE_H_

#include "daisykitsdk/common/types.h"
#include "daisykitsdk/graphs/core/node.h"
#include "daisykitsdk/graphs/core/utils.h"
#include "daisykitsdk/utils/visualizers/face_visualizer.h"

#include <chrono>
#include <memory>

namespace daisykit {
namespace graphs {

class FaceVisualizerNode : public Node {
 public:
  FaceVisualizerNode(const std::string& node_name,
                     NodeType node_type = NodeType::kSyncNode,
                     bool with_landmark = false)
      : Node(node_name, node_type) {
    with_landmark_ = with_landmark;
  }

  void Process(std::shared_ptr<Packet> in_packet,
               std::shared_ptr<Packet>& out_packet) {}

  void Tick() {
    WaitForData();

    std::map<std::string, PacketPtr> inputs;
    if (PrepareInputs(inputs) != 0) {
      std::cerr << GetNodeName() << ": Error on preparing inputs." << std::endl;
      return;
    }

    // Get new faces result
    // or take the last result
    std::shared_ptr<std::vector<daisykit::common::Face>> faces;
    if (inputs.count("faces") > 0) {
      faces = ParseFacePacket(inputs["faces"]);
      faces_ = faces;
    } else {
      faces = faces_;
    }

    // Get image
    cv::Mat img;
    Packet2CvMat(inputs["image"], img);

    // Clone image to draw on
    cv::Mat draw = img.clone();

    // Draw face to image
    if (faces != nullptr) {
      utils::visualizers::FaceVisualizer::DrawFace(draw, *faces,
                                                   with_landmark_);
    }

    cv::cvtColor(draw, draw, cv::COLOR_RGB2BGR);
    cv::imshow("Face detection", draw);
    cv::waitKey(1);
  }

  std::shared_ptr<std::vector<daisykit::common::Face>> ParseFacePacket(
      PacketPtr packet) {
    // Get data
    std::shared_ptr<void> data;
    utils::TimePoint timestamp;
    packet->GetData(data, timestamp);

    // Cast to faces
    std::shared_ptr<std::vector<daisykit::common::Face>> faces =
        std::static_pointer_cast<std::vector<daisykit::common::Face>>(data);

    return faces;
  }

 private:
  bool with_landmark_;
  std::shared_ptr<std::vector<daisykit::common::Face>> faces_;
};

}  // namespace graphs
}  // namespace daisykit

#endif