/* *********************************************************************************************************************
* Copyright 2026 Matthew Krueger <contact@matthewkrueger.com>
 * Original Source: https://github.com/Matthew-Krueger/simple-timer
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESSFOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************************************************/



#include <iostream>
#include <random>
#include <thread>

#include "timer.hpp"

using namespace std::chrono_literals;


static std::mt19937 gen(std::random_device{}());  // seeded once
static std::uniform_int_distribution<int> dist(1, 100);

void doLongTask() {
    std::this_thread::sleep_for(750ms);
}

void doLongTaskWithParam(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
}

int doLongTaskWithReturn() {
    std::this_thread::sleep_for(1s);
    return dist(gen);
}

int doLongTaskWithReturnAndParam(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
    return dist(gen);
}

void demo();

int main() {

    std::cout << "Timer Demo" << std::endl;
    std::cout << "=========" << std::endl;

    demo();

    return 0;

}

void demo() {
    std::cout << "\n=== Simple Timer Full Demo ===\n\n";

    // We'll use these tasks for the demo
    auto voidNoParamTask = [] {
        doLongTask();
    };

    auto param = 750ms;

    // Time all variants once
    const auto voidNoParamResult = timer::time(voidNoParamTask);
    const auto voidWithParamResult = timer::time([param] {
        return doLongTaskWithParam(param);
    });
    const auto returnNoParamResult = timer::time(doLongTaskWithReturn);
    const auto returnWithParamResult = timer::time([param] {
        return doLongTaskWithReturnAndParam(param);
    });

    // 1. Recommended & simplest: direct double seconds (highest precision)
    std::cout << "[1] Direct access (recommended) - fractional seconds:\n";
    std::cout << "    Void, no param: " << voidNoParamResult.duration.count() << " seconds\n";
    std::cout << "    Void, curried param: " << voidWithParamResult.duration.count() << " seconds\n";
    std::cout << "    Return, no param: " << returnNoParamResult.duration.count() << " seconds (returned " << returnNoParamResult.functionResult << ")\n";
    std::cout << "    Return, curried param: " << returnWithParamResult.duration.count() << " seconds (returned " << returnWithParamResult.functionResult << ")\n\n";

    // 2. Full-precision custom units (no truncation)
    std::cout << "[2] Full-precision custom units (no truncation) - using return with curried param example:\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::picoseconds>().as<double>()  << " picoseconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::nanoseconds>().as<double>()  << " nanoseconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::microseconds>().as<double>() << " microseconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::milliseconds>().as<double>() << " milliseconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::seconds>().as<double>()      << " seconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::minutes>().as<double>()      << " minutes\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::hours>().as<double>()        << " hours\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::days>().as<double>()         << " days\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::weeks>().as<double>()        << " weeks\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::years>().as<double>()        << " years\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::decades>().as<double>()      << " decades\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::centuries>().as<double>()    << " centuries\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::millennia>().as<double>()    << " millennia\n\n";

    // 3. Standard std::chrono integer units → shows truncation
    std::cout << "[3] Standard chrono integer units (truncates fractions) - using return with curried param example:\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::nanoseconds>().as<int64_t>()   << " ns (truncated)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::microseconds>().as<int64_t>()  << " µs (truncated)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::milliseconds>().as<int64_t>() << " ms (truncated)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::seconds>().as<int64_t>()       << " s  (truncated)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::minutes>().as<int64_t>()       << " min (truncated)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<std::chrono::hours>().as<int64_t>()         << " h   (truncated)\n\n";

    // 4. High-resolution capture → display in desired unit
    std::cout << "[4] High-resolution capture → display in desired unit - using return with curried param example:\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::microseconds>().as<double>() / 1'000'000.0
              << " seconds (from µs)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::milliseconds>().as<double>() / 1000.0
              << " seconds (from ms)\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::milliseconds>().as<double>()
              << " milliseconds\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::microseconds>().as<double>() / 1000.0
              << " milliseconds (from µs)\n\n";

    // 5. Fun with long units (for very long simulations)
    std::cout << "[5] Extreme units (for long-running jobs) - using return with curried param example:\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::decades>().as<double>()    << " decades\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::centuries>().as<double>()  << " centuries\n";
    std::cout << "    " << returnWithParamResult.getDurationView<timer::millennia>().as<double>()  << " millennia\n\n";

    std::cout << "=== End of Demo ===\n";
}