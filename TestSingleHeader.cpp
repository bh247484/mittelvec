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

    while (keepRunning) {
        std::string input;
        if (std::cin >> input) {
            if (input == "x") {
                keepRunning = false;
            } else if (input == "k") {
                samplePack.triggerSample("nbc-chimes");
            } else if (input == "l") {
                samplePack.triggerSample("nbc-chimes-high");
            } else if (input == "j") {
                samplePack.triggerSample("nbc-chimes-low");
            } else if (input == "b") {
                samplePack.triggerSample("blip");
            } else if (input == "n") {
                samplePack.triggerSample("blip-high");
            } else if (input == "v") {
                samplePack.triggerSample("blip-low");
            } else if (input == "y") {
                samplePack.triggerSample("perc2");
            } else if (input == "r") {
                samplePack.triggerSample("perc3");
            } else if (input == "t") {
                samplePack.triggerSample("perc");
            } else if (input == "q") {
                cueOrchestrator.playCue("cue-1");
            } else if (input == "w") {
                cueOrchestrator.playCue("cue-2");
            } else if (input == "e") {
                cueOrchestrator.stopCue();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    engine.stop();
    return 0;
}
