//
// Created by jack on 4/20/2025.
//

#ifndef STEPTIMERPROFILER_H
#define STEPTIMERPROFILER_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

template<bool enable = true>
class StepTimerProfiler {
public:
    StepTimerProfiler(int reportEveryNFrames = 50000, double spikeMultiplier = 2.0)
        : reportInterval(reportEveryNFrames), spikeThresholdMultiplier(spikeMultiplier) {
        lastTimePoint = std::chrono::high_resolution_clock::now();
    }

    void checkTime(const std::string& stepName) {

        if constexpr (enable) {
            using namespace std::chrono;

            auto now = high_resolution_clock::now();
            double deltaMs = duration<double, std::milli>(now - lastTimePoint).count();
            lastTimePoint = now;

            StepStats& stats = steps[stepName];
            stats.totalTime += deltaMs;
            stats.count++;
            stats.maxTime = std::max(stats.maxTime, deltaMs);

            double avg = stats.totalTime / stats.count;
            if (deltaMs > avg * spikeThresholdMultiplier && stats.count > 5) {
                stats.spikeCount++;
            }

            frameStepCount++;
            if (frameStepCount >= reportInterval) {
                printReport();

                reset();
                lastTimePoint = std::chrono::high_resolution_clock::now();
            }
        }
        
    }

private:
    struct StepStats {
        double totalTime = 0;
        int count = 0;
        double maxTime = 0;
        int spikeCount = 0;

        double avgTime() const {
            return count > 0 ? totalTime / count : 0;
        }
    };

    std::unordered_map<std::string, StepStats> steps;
    std::chrono::high_resolution_clock::time_point lastTimePoint;

    int reportInterval;
    int frameStepCount = 0;
    double spikeThresholdMultiplier;

    void printReport() {


        if (steps.empty()) return;

        // std::vector<std::pair<std::string, StepStats>> sortedSteps(steps.begin(), steps.end());
        // std::sort(sortedSteps.begin(), sortedSteps.end(),
        //           [](const auto& a, const auto& b) {
        //               return a.second.avgTime() > b.second.avgTime();
        //           });

        std::vector<std::pair<std::string, StepStats>> sortedMaxs(steps.begin(), steps.end());
        std::sort(sortedMaxs.begin(), sortedMaxs.end(),
                  [](const auto& a, const auto& b) {
                      return a.second.maxTime > b.second.maxTime;
                  });

        // std::cout << "\n=== Step Timing Report ===\n";
        // std::cout << std::fixed << std::setprecision(2);
        //
        // for (const auto& [name, stats] : sortedSteps) {
        //     std::cout << "Step: " << name
        //               << " | Avg: " << stats.avgTime() << " ms"
        //               << " | Max: " << stats.maxTime << " ms"
        //               << " | Spikes: " << stats.spikeCount
        //               << "\n";
        // }

        std::cout << "\n=== Maxes: ===\n";
        std::cout << std::fixed << std::setprecision(2);

        for (const auto& [name, stats] : sortedMaxs) {
            std::cout << "Step: " << name
                      << " | Avg: " << stats.avgTime() << " ms"
                      << " | Max: " << stats.maxTime << " ms"
                      << " | Spikes: " << stats.spikeCount
                      << "\n";
        }
        std::cout << "===========================\n";
    }

    void reset() {
        frameStepCount = 0;
        steps.clear();
    }
};

#endif //STEPTIMERPROFILER_H
