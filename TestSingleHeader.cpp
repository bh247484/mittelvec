#define MITTELVEC_IMPLEMENTATION
#include "dist/mittelvec.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <vector>
#include <string>

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 1;
const float SAMPLE_RATE = 44100.0f;

std::atomic<bool> keepRunning(true);

int main() {
    std::signal(SIGINT, [](int) { keepRunning = false; });

    // Basic setup
    MittelVec::AudioContext globalContext = { BUFFER_SIZE, NUM_CHANNELS, SAMPLE_RATE };
    MittelVec::Engine engine(globalContext);
    engine.start();

    MittelVec::AudioGraph& graph = engine.graph;

    std::cout << "MittelVec Single Header Test (Mirroring main.cpp)" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Samples: k, l, j, b, n, v, y, r, t" << std::endl;
    std::cout << "  Music Cues: q (cue-1), w (cue-2), e (stop)" << std::endl;
    std::cout << "  Other: x (exit)" << std::endl;

    // Declare SamplePackItems.
    std::vector<MittelVec::SamplePackItem> samplePackItems = {
        {"nbc-chimes", "nbc-chimes.wav", 6, false, 5.0, 0},
        {"nbc-chimes-high", "nbc-chimes.wav", 6, false, 5.0, 7},
        {"nbc-chimes-low", "nbc-chimes.wav", 6, false, 5.0, -7},
        {"blip", "blip.wav", 6, false, 5.0, 0},
        {"blip-high", "blip.wav", 6, false, 5.0, 8},
        {"blip-low", "blip.wav", 6, false, 5.0, -5},
        {"perc2", "perc.wav", 6, false, 5.0, 2},
        {"perc3", "perc.wav", 6, false, 5.0, -3},
        {"perc", "perc.wav", 6, false, 5.0, 0},
    };

    std::string samplesDir = "/Users/bh/Documents/game-audio/middleware-exp/sounds/";
    MittelVec::SamplePack samplePack(graph, samplePackItems, samplesDir);

    std::vector<MittelVec::MusicCue> musicCues = {
        {"cue-1", "cue-1.wav", true},
        {"cue-2", "cue-2.wav", true},
    };

    std::string cuesDir = "/Users/bh/Documents/game-audio/middleware-exp/cues/";
    MittelVec::MusicCueOrchestrator cueOrchestrator(graph, musicCues, cuesDir);

    std::unordered_map<char, std::function<void()>> inputMap = {
        // Killswitch
        {'x', [&]() { keepRunning = false; }},
        
        // Samples
        {'k', [&]() { samplePack.triggerSample("nbc-chimes"); }},
        {'l', [&]() { samplePack.triggerSample("nbc-chimes-high"); }},
        {'j', [&]() { samplePack.triggerSample("nbc-chimes-low"); }},
        {'b', [&]() { samplePack.triggerSample("blip"); }},
        {'n', [&]() { samplePack.triggerSample("blip-high"); }},
        {'v', [&]() { samplePack.triggerSample("blip-low"); }},
        {'y', [&]() { samplePack.triggerSample("perc2"); }},
        {'r', [&]() { samplePack.triggerSample("perc3"); }},
        {'t', [&]() { samplePack.triggerSample("perc"); }},

        // Music Cues
        {'q', [&]() { cueOrchestrator.playCue("cue-1"); }},
        {'w', [&]() { cueOrchestrator.playCue("cue-2"); }},
        {'e', [&]() { cueOrchestrator.stopCue(); }}
    };

    while (keepRunning) {
        char input;
        if (std::cin >> input) {
            // Check if the key exists in our map
            if (inputMap.count(input)) {
                inputMap[input](); // Execute the associated lambda
            } else {
                std::cout << "Unknown command: " << input << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Shorter sleep for better latency
    }

    engine.stop();
    return 0;
}
