#define _USE_MATH_DEFINES
#include "../CVSDDelayCore.h"
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>
#include <map>

const double SAMPLE_RATE = 44100.0;
const int LENGTH_SEC = 2;
const int NUM_SAMPLES = static_cast<int>(SAMPLE_RATE * LENGTH_SEC);
const int BLOCK_SIZE = 128;

struct Scenario {
    std::string name;
    std::map<ParamID, double> params;
};

void runScenario(const std::string& signalName, const std::vector<float>& inputData, const Scenario& scenario) {
    CVSDDelayCore core;
    core.prepare(SAMPLE_RATE);
    core.setRandomSeed(1337);

    // Apply params
    for (const auto& kv : scenario.params) {
        core.setParameter(kv.first, kv.second);
    }

    std::vector<float> outputData(NUM_SAMPLES, 0.0f);

    for (int i = 0; i < NUM_SAMPLES; i += BLOCK_SIZE) {
        int samplesToProcess = std::min(BLOCK_SIZE, NUM_SAMPLES - i);

        const float* inputs[1] = { &inputData[i] };
        float* outputs[1] = { &outputData[i] };

        core.process(inputs, outputs, 1, samplesToProcess);
    }

    std::string outPath = "vectors/cpp_out_" + signalName + "_" + scenario.name + ".raw";
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file: " << outPath << std::endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(outputData.data()), outputData.size() * sizeof(float));
    std::cout << "Saved: " << outPath << std::endl;
}

std::vector<float> readInput(const std::string& name) {
    std::string path = "vectors/input_" + name + ".raw";
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open " << path << std::endl;
        return std::vector<float>(NUM_SAMPLES, 0.0f);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<float> data(size / sizeof(float));
    if (file.read(reinterpret_cast<char*>(data.data()), size)) {
        return data;
    }
    return std::vector<float>(NUM_SAMPLES, 0.0f);
}

int main() {
    std::vector<float> impulse = readInput("impulse");
    std::vector<float> chirp = readInput("chirp");
    std::vector<float> noise = readInput("noise");

    std::vector<Scenario> scenarios = {
        {
            "default",
            {
                {ParamID::feedback, 0.0},
                {ParamID::mix, 1.0},
                {ParamID::clockJitter, 0.0}
            }
        },
        {
            "companded_wobble",
            {
                {ParamID::feedback, 0.6},
                {ParamID::wobbleDepth, 10.0},
                {ParamID::circuitType, 1.0},
                {ParamID::clockJitter, 0.0}
            }
        },
        {
            "reverse",
            {
                {ParamID::reverseMode, 1.0},
                {ParamID::clockJitter, 0.0}
            }
        }
    };

    struct Signal {
        std::string name;
        const std::vector<float>* data;
    };

    std::vector<Signal> signals = {
        {"impulse", &impulse},
        {"chirp", &chirp},
        {"noise", &noise}
    };

    for (const auto& signal : signals) {
        for (const auto& scenario : scenarios) {
            runScenario(signal.name, *signal.data, scenario);
        }
    }

    return 0;
}
