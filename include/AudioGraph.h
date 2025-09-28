
#pragma once
#include "AudioNode.h"
#include "AudioBuffer.h"
#include "AudioContext.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace Middleman {

class AudioGraph {
public:
    AudioGraph(const AudioContext& context);

    // int addNode(std::unique_ptr<AudioNode> node);
    template <typename NodeType, typename... Args>
    std::pair<int, NodeType*> addNode(Args&&... args);
    // std::vector<int> addNodes(std::vector<std::shared_ptr<AudioNode>> nodes);
    void removeNode(int nodeId);
    void connect(int sourceNodeId, int destNodeId);
    void disconnect(int sourceNodeId, int destNodeId);
    void processGraph(AudioBuffer& outputBuffer);

    void updateProcessOrder();
    AudioContext audioContext;
    std::unordered_map<int, std::unique_ptr<AudioNode>> nodes;
    std::unordered_map<int, std::vector<int>> connections;
    std::unordered_map<int, std::vector<int>> reverseConnections;
    int nextNodeId;
    std::vector<int> processOrder;
    bool isGraphDirty;
};

} // namespace
