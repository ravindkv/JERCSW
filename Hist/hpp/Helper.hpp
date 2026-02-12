#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include "Rtypes.h" // Long64_t

class Helper {
public:
    static inline void printBanner(const std::string& title) {
        std::cout << "\n--------------------------------------\n";
        std::cout << " " << title << "\n";
        std::cout << "--------------------------------------\n";
    }

    // Print progress table header and return cadence for "every 1%" updates.
    // - For nentries >= 100: everyN = nentries/100 (≈1%)
    // - For nentries < 100:  everyN = 1 (print every event; previously printed nothing)
    static inline Long64_t initProgress(Long64_t nentries) {
        std::cout << "\nStarting loop over " << nentries << " entries\n";
        std::cout << "---------------------------\n";
        std::cout << std::setw(10) << "Progress" << std::setw(10) << "Time" << "\n";
        std::cout << "---------------------------\n";

        if (nentries <= 0) return 1;
        const Long64_t everyN = (nentries >= 100) ? (nentries / 100) : 1;
        return (everyN > 0) ? everyN : 1;
    }

    // Print progress every N entries (cadence computed once outside the loop).
    static inline void printProgressEveryN(
        Long64_t jentry,
        Long64_t nentries,
        Long64_t everyN,
        std::chrono::time_point<std::chrono::high_resolution_clock>& startClock,
        double& totTime
    ) {
        if (nentries <= 0) return;
        if (everyN <= 0) return;
        if (jentry % everyN != 0) return;

        const auto currentTime = std::chrono::high_resolution_clock::now();
        totTime += std::chrono::duration<double>(currentTime - startClock).count();

        const int sec = static_cast<int>(totTime) % 60;
        const int min = static_cast<int>(totTime) / 60;

        const Long64_t pct = (100 * jentry) / nentries;

        std::cout << std::setw(5) << pct << "% "
                  << std::setw(5) << min << "m " << sec << "s\n";

        startClock = currentTime;
    }
};

