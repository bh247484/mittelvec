#include "../include/AudioGraph.h"
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <iostream>

namespace Middleman {

AudioGraph::AudioGraph(const AudioContext& context)
  : audioContext(context), nextNodeId(0), isGraphDirty(true) {}

void AudioGraph::removeNode(int nodeId) {
  if (nodes.find(nodeId) == nodes.end()) return;

  nodes.erase(nodeId);
  connections.erase(nodeId);
  reverseConnections.erase(nodeId);

  for (auto& pair : connections) {
    auto& destinations = pair.second;
    destinations.erase(std::remove(destinations.begin(), destinations.end(), nodeId), destinations.end());
  }

  for (auto& pair : reverseConnections) {
    auto& sources = pair.second;
    sources.erase(std::remove(sources.begin(), sources.end(), nodeId), sources.end());
  }

  isGraphDirty = true;
}

void AudioGraph::connect(int sourceNodeId, int destNodeId) {
  if (nodes.find(sourceNodeId) == nodes.end() || nodes.find(destNodeId) == nodes.end()) {
    return;
  }

  connections[sourceNodeId].push_back(destNodeId);
  reverseConnections[destNodeId].push_back(sourceNodeId);
  isGraphDirty = true;
}

void AudioGraph::disconnect(int sourceNodeId, int destNodeId) {
  if (connections.count(sourceNodeId)) {
    auto& destinations = connections[sourceNodeId];
    destinations.erase(std::remove(destinations.begin(), destinations.end(), destNodeId), destinations.end());
  }

  if (reverseConnections.count(destNodeId)) {
    auto& sources = reverseConnections[destNodeId];
    sources.erase(std::remove(sources.begin(), sources.end(), sourceNodeId), sources.end());
  }

  isGraphDirty = true;
}

void AudioGraph::updateProcessOrder() {
  processOrder.clear();
  if (nodes.empty()) {
    isGraphDirty = false;
    return;
  }

  // Map to store in-degrees of each node
  std::unordered_map<int, int> inDegree;
  for (const auto& pair : nodes) {
    inDegree[pair.first] = 0;
  }

  // Calculate in-degrees
  for (const auto& pair : connections) {
    for (int destNodeId : pair.second) {
      inDegree[destNodeId]++;
    }
  }

  // Initialize queue with nodes having an in-degree of 0 (our source nodes)
  std::queue<int> queue;
  for (const auto& pair : inDegree) {
    if (pair.second == 0) {
      queue.push(pair.first);
    }
  }

  // Process nodes using Kahn's algorithm
  while (!queue.empty()) {
    int nodeId = queue.front();
    queue.pop();
    processOrder.push_back(nodeId);

    // Decrement inDegrees of neighboring nodes
    if (connections.count(nodeId)) {
      for (int neighbor : connections[nodeId]) {
        inDegree[neighbor]--;
        if (inDegree[neighbor] == 0) {
          queue.push(neighbor);
        }
      }
    }
  }

  // Check for cycles. If a cycle exists, the graph is invalid.
  if (processOrder.size() != nodes.size()) {
    std::cerr << "Error: Cycle detected in the audio graph. Audio processing will be stopped." << std::endl;
    processOrder.clear(); // Clear the invalid processing order
  }

  isGraphDirty = false;
}


void AudioGraph::processGraph(AudioBuffer& outputBuffer) {
  // If the graph structure has changed, recalculate the processing order.
  if (isGraphDirty) {
    updateProcessOrder();
  }

  // If the process order is empty (and there are nodes), it means a cycle was detected.
  if (processOrder.empty() && !nodes.empty()) {
    outputBuffer.clear();
    return; // Output silence if graph is invalid
  }

  // Process each node in the topologically sorted order
  for (int nodeId : processOrder) {
    auto& node = nodes[nodeId];

    // Find inputs for the current node from its predecessors' output buffers
    std::vector<const AudioBuffer*> inputs;
    if (reverseConnections.count(nodeId)) {
      for (int sourceId : reverseConnections[nodeId]) {
        if (nodes.count(sourceId)) {
          inputs.push_back(&(nodes[sourceId]->outputBuffer));
        }
      }
    }
    // Process the node
    node->process(inputs, node->outputBuffer);
  }

  // Sum the outputs of all "terminal" nodes (nodes with no outgoing connections)
  outputBuffer.clear();
  for (auto const& [nodeId, node] : nodes) {
    if (connections.find(nodeId) == connections.end() || connections[nodeId].empty()) {
      for (int i = 0; i < outputBuffer.size(); ++i) {
        outputBuffer[i] += node->outputBuffer[i];
      }
    }
  }
}

} // namespace
