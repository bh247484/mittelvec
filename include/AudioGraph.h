
#pragma once
#include "AudioNode.h"
#include "AudioBuffer.h"
#include "AudioContext.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace MittelVec {

class AudioGraph {
public:
    AudioGraph(const AudioContext& context);

    template <typename NodeType, typename... Args>
    std::pair<int, NodeType*> addNode(Args &&...args)
    {
    int id = nextNodeId++;
    auto node = std::make_unique<NodeType>(audioContext, std::forward<Args>(args)...);
    nodes[id] = std::move(node);
    isGraphDirty = true;
    return std::make_pair(id, static_cast<NodeType*>(nodes[id].get()));
    }

    void removeNode(int nodeId);
    void connect(int sourceNodeId, int destNodeId);
    void disconnect(int sourceNodeId, int destNodeId);
    void processGraph(AudioBuffer& outputBuffer);
    void setAudioContext(AudioContext newContext);

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
