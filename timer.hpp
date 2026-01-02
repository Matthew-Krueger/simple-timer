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


#pragma once
#ifndef MCKRUEG_TIMER_HPP
#define MCKRUEG_TIMER_HPP
#include <chrono>
#include <iostream>
#include <type_traits>

#undef BUILD_WITH_MPI_FLAG
#ifdef BUILD_WITH_MPI
#define BUILD_WITH_MPI_FLAG true
#include <mpi.h>

// This validates the function exists
static_assert([] {
                  using F = decltype(&::MPI_Wtime);
                  return sizeof(F) > 0;
              }(),
              "MPI_Wtime declaration missing - check MPI installation and linking");
#else
#define BUILD_WITH_MPI_FLAG false
// create a fake MPI_Wtime forward declaration. This should link without warning as it satisfies the C linkage
// requirement so if constexpr doesn't hurl.
// This is a weak declaration, and as such will not require the linkage of any function
// it only exists to satisfy the compiler step. The linker does not give a tinkers darn.

extern "C" {
#if defined(__GNUC__) || defined(__clang__)
__attribute__((weak))
#endif
double MPI_Wtime();
}
#endif


namespace timer {

    using picoseconds = std::chrono::duration<double, std::ratio<1, 1000000000000>>;
    using nanoseconds = std::chrono::duration<double, std::ratio<1, 1000000000>>;
    using microseconds = std::chrono::duration<double, std::ratio<1, 1000000>>;
    using milliseconds = std::chrono::duration<double, std::ratio<1, 1000>>;  // 1 unit = 1 ms
    using seconds = std::chrono::duration<double, std::ratio<1>>;
    using minutes = std::chrono::duration<double, std::ratio<60>>;
    using hours = std::chrono::duration<double, std::ratio<3600>>;
    using days = std::chrono::duration<double, std::ratio<86400>>;
    using weeks = std::chrono::duration<double, std::ratio<604800>>;
    using years = std::chrono::duration<double, std::ratio<31556952>>;
    using decades = std::chrono::duration<double, std::ratio<315569520>>;
    using centuries = std::chrono::duration<double, std::ratio<3155695200>>;
    using millennia = std::chrono::duration<double, std::ratio<31556952000>>;

    // ReSharper disable once CppRedundantTemplateArguments
    // A duration, stored in double precision, pegged to 1 value with zero exponent is 1 second
    using Duration = std::chrono::duration<double, std::ratio<1, 1> >; // seconds, double precision

    // Defines the clock to use for the standard library. By default, we will use the steady clock
    // This is because it is guaranteed to be a steady clock, where as std::chrono::high_resolution_clock or system_clock
    // is not guaranteed to be steady and may jump
    // This can be changed as you want, and high_resolution_clock may be more accurate,
    // however, by default the best choice is steady_clock unless you can validate that high_resolution_clock is
    // steady on your platform
    using Clock = std::chrono::steady_clock;

    // Stores a point in time using our clock and our duration.
    using TimePoint = std::chrono::time_point<Clock, Duration>;


    // Trait: Is the type a std::chrono::duration?
    template<typename T>
    struct is_chrono_duration : std::false_type {
    };

    // enforce that it is compatable
    template<typename Rep, typename Period>
    struct is_chrono_duration<std::chrono::duration<Rep, Period> > : std::true_type {
    };

    // Now constrain TimeView based on this
    template<typename ReturnDuration,
        std::enable_if_t<is_chrono_duration<ReturnDuration>::value, int> = 0>
    struct TimeView {
        /**
         * The duration of the view. This may be in any unit compatible with std::chrono.
         */
        ReturnDuration duration;

        // composable type
        inline decltype(auto) count() const { return duration.count(); }

        /**
         * @brief Casts the time point to the specified unit.
         *
         * Casts the duration to the specified type. The type is given in the type relation embedded within the std::chrono unit.
         * For instance, if duration is of type std::chrono::duration<std::chrono::seconds, std::ratio<1, 1>>, and Rep is uint64_t,
         * then as<uint64_t>() will return the number of seconds as a uint64_t. Rounding/truncating may occur per C++ spec.
         * Likewise if it is a double, then as<double>() will return the number of seconds as a double. TRUNCATION MAY STILL OCCUR BASED ON PRECISION OF DURATION, DETERMINED BY COMPOSITION!
         *
         * @note TRUNCATION MAY STILL OCCUR BASED ON PRECISION OF DURATION, DETERMINED BY COMPOSITION!
         * @note Rounding may occur to achieve specified accuracy, per C++ spec on std::chrono
         * @note For small units but large times, this may overflow or truncate. This behavior is not monitored. It is the caller's responsibility to ensure reasonable estimates about the size are made. For example, don't expect picoseconds when operating on a 6-month process.
         * @tparam Rep The type to convert to. For example, a uint64_t
         * @return The duration, cast as type Rep.
         */
        template<typename Rep>
        constexpr Rep as() const {
            if constexpr (std::is_same_v<Rep, ReturnDuration>) {
                return duration;
            } else if constexpr (std::is_floating_point_v<Rep> || std::is_integral_v<Rep>) {
                return static_cast<Rep>(duration.count());
            } else {
                static_assert(false, "Invalid type for as() function");
                return {};
            }
        }

        // Operators passthrough (e.g., for composability)
        inline operator ReturnDuration() const { return duration; }
        inline ReturnDuration &&unwrap() const { return std::move(duration); }
    };

    template<typename T>
    /**
     * Represents the result of a function execution along with the time taken, measured in @link Duration
     *
     * @tparam T Type of the function result.
     */
    struct TimeResult {
        /**
         * Stores the result of the executed function.
         * This value represents the output of the function being timed.
         *
         * Ideally, this value should be move-constructable in order to enable copy elision and reduce overhead.
         * Though this behavior is only guaranteed to occur in C++17 and above.
         */
        T functionResult;

        /**
         * Represents a time duration measured in seconds with double-precision.
         * This type is used to store or measure elapsed time in seconds, leveraging
         * `std::chrono::duration`.
         *
         * See @link Duration
         */
        Duration duration;

        /**
         * @brief Casts the duration to a specified type.
         * Default: double precision seconds
         * @note THIS OPERATION WILL TRUNCATE!
         * @tparam ReturnDuration The type to which you want to duration cast. NOTE: This operation will truncate!
         * @return The truncated value, in a duration
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline ReturnDuration getDuration() const {
            return std::chrono::duration_cast<ReturnDuration>(duration);
        }

        /**
         * @brief Casts the duration to a specified type, and returns the count.
         * @note Truncates/rounds to units.
         * For example, a duration of 1.3 seconds will truncate to 1 second if used with std::chrono::seconds
         * @tparam ReturnDuration The target duration. DEFAULT: double precision seconds
         * @return The numeric count (e.g., uint64_t for microseconds).
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline auto getDurationCount() const -> decltype(getDuration<ReturnDuration>().count()) {
            return getDuration<ReturnDuration>().count();
        }

        /**
         * @brief Returns a TimeView of the duration, in units ReturnDuration.
         * @note This will truncate to the specified unit, no rounding will occur unless the default cast operator performs it. That means if you ask for seconds, you get seconds. Not 1.3 seconds. Math.floor(seconds).
         *
         * By Default this returns a full double precision as seconds.
         *
         * @example TimeResult<T>.getDurationView<std::chrono::seconds>().as<double>();
         *
         * @tparam ReturnDuration The unit of time you want back, for example, std::chrono::seconds
         * @return The TimeView of the duration.
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline auto getDurationView() const -> TimeView<ReturnDuration> {
            return TimeView<ReturnDuration>{getDuration<ReturnDuration>()};
        }
    };

    /**
     * Specialization of TimeResult for void functions.
     * It does not include a functionResult member.
     */
    template<>
    struct TimeResult<void> {
        Duration duration;

        /**
         * @brief Casts the duration to a specified type.
         * Default: double precision seconds
         * @note THIS OPERATION WILL TRUNCATE!
         * @tparam ReturnDuration The type to which you want to duration cast. NOTE: This operation will truncate!
         * @return The truncated value, in a duration
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline ReturnDuration getDuration() const {
            return std::chrono::duration_cast<ReturnDuration>(duration);
        }

        /**
         * @brief Casts the duration to a specified type, and returns the count.
         * @note Truncates/rounds to units.
         * For example, a duration of 1.3 seconds will truncate to 1 second if used with std::chrono::seconds
         * @tparam ReturnDuration The target duration. DEFAULT: double precision seconds
         * @return The numeric count (e.g., uint64_t for microseconds).
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline auto getDurationCount() const -> decltype(getDuration<ReturnDuration>().count()) {
            return getDuration<ReturnDuration>().count();
        }

        /**
         * @brief Returns a TimeView of the duration, in units ReturnDuration.
         * @note This will truncate to the specified unit, no rounding will occur unless the default cast operator performs it. That means if you ask for seconds, you get seconds. Not 1.3 seconds. Math.floor(seconds).
         *
         * By Default this returns a full double precision as seconds.
         *
         * @example TimeResult<T>.getDurationView<std::chrono::seconds>().as<double>();
         *
         * @tparam ReturnDuration The unit of time you want back, for example, std::chrono::seconds
         * @return The TimeView of the duration.
         */
        template<typename ReturnDuration = std::chrono::duration<double>>
        inline auto getDurationView() const -> TimeView<ReturnDuration> {
            return TimeView<ReturnDuration>{getDuration<ReturnDuration>()};
        }
    };

    // Deduction guide for non-void TimeResult
    template<typename T, typename Dur>
    TimeResult(T &&, Dur &&) -> TimeResult<std::decay_t<T> >;

    // Deduction guide for void TimeResult (from duration only)
    template<typename Dur>
    TimeResult(Dur &&) -> TimeResult<void>;

    /**
     * The SFINAE Return type of a NonVoid function.
     */
    template<typename FuncToTime>
    using NonVoidTimeResult = std::enable_if_t<!std::is_void_v<std::invoke_result_t<FuncToTime> >,
        TimeResult<std::invoke_result_t<FuncToTime> > >;

    /**
     * The SFINAE Return type of a Void function.
     */
    template<typename FuncToTime>
    using VoidTimeResult = std::enable_if_t<std::is_void_v<std::invoke_result_t<FuncToTime> >,
        TimeResult<void> >;

    /**
     * @brief Times a function, returning a TimeResult<T>.
     * The result of the function may be accessed via the member variable TimeResult<T>.functionResult;
     * For best performance this value should be move constructable (i.e., support std::move by supporting a move constructor [constructor taking an r value reference]).
     * This is determined by the type of function being timed. If the function returns void, no result is stored.
     * The compiler will invoke the proper version of the time function based on the type.
     *
     * The time result may be accessed in multiple ways. You can get the default duration with TimeResult<T>.duration;
     * However, several cast methods have been built in for you.
     * For instance, you can get a std::chrono::duration type in the unit you specify by calling TimeResult<T>.getDuration<U>(); where U is the std::chrono::U unit of time.
     * You can also get the count (which rounds or truncates) of this by calling TimeResult<T>.getDurationCount<U>();
     *
     * Lastly, you can aquire a TimeView to use a more python like interface:
     * V timeResult = TimeResult<T>.getDurationView<U>().as<V>();, where U is the unit (std::chrono::U) and V is the type you want to cast to, for instance a double.
     * So, for example, to get a double representing the duration in seconds, you can use TimeResult<T>.getDurationView<std::chrono::seconds>().as<double>();
     *
     * @tparam FuncToTime The type of function to time. This will usually be a lambda with curried data but may be a function pointer to a void function.
     * @param toTime The function to time.
     * @return A TimeResult containing the duration of the function execution, and the function result.
     */
    template<typename FuncToTime>
    NonVoidTimeResult<FuncToTime> time(FuncToTime toTime);

    /**
     * @brief Times a function, returning a TimeResult<void>.
     *
     * No return value is stored, and a compiler error will occur if you attempt to access it.
     * This is determined by the type of function being timed. If the function returns void, no result is stored.
     * The compiler will invoke the proper version of the time function based on the type.
     *
     * The time result may be accessed in multiple ways. You can get the default duration with TimeResult<T>.duration;
     * However, several cast methods have been built in for you.
     * For instance, you can get a std::chrono::duration type in the unit you specify by calling TimeResult<T>.getDuration<U>(); where U is the std::chrono::U unit of time.
     * You can also get the count (which rounds or truncates) of this by calling TimeResult<T>.getDurationCount<U>();
     *
     * Lastly, you can aquire a TimeView to use a more python like interface:
     * V timeResult = TimeResult<T>.getDurationView<U>().as<V>();, where U is the unit (std::chrono::U) and V is the type you want to cast to, for instance a double.
     * So, for example, to get a double representing the duration in seconds, you can use TimeResult<T>.getDurationView<std::chrono::seconds>().as<double>();
     *
     * @tparam FuncToTime The type of function to time. This will usually be a lambda with curried data but may be a function pointer to a void function.
     * @param toTime The function to time.
     * @return A TimeResult containing the duration of the function execution, and the function result.
     */
    template<typename FuncToTime>
    VoidTimeResult<FuncToTime> time(FuncToTime toTime);

    /**
     * Represents a utility to measure the elapsed time between its instantiation and when the measurement ends.
     * Primarily used for timing the duration of scoped blocks of code or specific function executions.
     *
     * @note This cannot be externally invoked. Use the global time function instead: `timer::time(FuncToTime toTime)`
     */
    class Timer {
    public:
        ~Timer() {
            // get our end time
            TimePoint endTimePoint;

            if constexpr (BUILD_WITH_MPI_FLAG) {
                endTimePoint = TimePoint(Duration(MPI_Wtime()));
            } else {
                endTimePoint = std::chrono::time_point_cast<Duration>(Clock::now());
            }

            *m_TimeReference = endTimePoint - m_StartTimePoint;
        }

    private:
        /**
         * When called with a weak pointer, this will write the time elapsed, in microseconds, to m_TimeReference.
         * If m_TimeReference no longer is in scope, nothing will happen.
         * @param duration A pointer in which to write the resultant time. If it goes out of scope, nothing will be written
         */
        inline explicit Timer(Duration *duration) : m_TimeReference(duration) {
            if constexpr (BUILD_WITH_MPI_FLAG) {
                m_StartTimePoint = TimePoint(Duration(MPI_Wtime()));
            } else {
                m_StartTimePoint = std::chrono::time_point_cast<Duration>(Clock::now());
            }
        }

        /**
         * Contains a start timepoint, long is in microseconds, double is in seconds
         */
        TimePoint m_StartTimePoint = TimePoint(Duration(0.0));

        /**
         * Contains a pointer to the duration reference, which is used to store the elapsed time.
         * This allows for the time to be accessed outside the scope of the Timer object.
         *
         * The timer class assumes this will NOT go out of scope and become dangling.
         * There is no runtime check for this. To prevent this, the Timer constructor is private preventing instantiation.
         */
        Duration *m_TimeReference;

        // mark non-void time as a friend so it can instantiate the timer
        template<typename FuncToTime>
        friend NonVoidTimeResult<FuncToTime> time(FuncToTime);

        // mark void time as a friend so it can instantiate the timer
        template<typename FuncToTime>
        friend VoidTimeResult<FuncToTime> time(FuncToTime);
    };

    template<typename FuncToTime>
    inline NonVoidTimeResult<FuncToTime> time(FuncToTime toTime) {
        using ResultType = std::invoke_result_t<FuncToTime>;

        Duration duration;
        ResultType result = [&] {
            Timer timer(&duration);
            return toTime();
        }();

        // We use an IILE here to avoid multiple constructions of the ReturnType, and to preserve it.
        // Because NRVO will take care of it automatically, we are just effectively telling the compiler
        // YES, in fact, I DO want this optimization, even if it doesn't immediately make sense in context.
        return TimeResult<ResultType>{
            static_cast<ResultType &&>(result),
            duration
        };
    }

    template<typename FuncToTime>
    inline VoidTimeResult<FuncToTime> time(FuncToTime toTime) {
        TimeResult<void> result{};
        {
            Timer timer(&result.duration);
            toTime(); // Just execute the function
        }
        return result;
    }
}


#endif //MCKRUEG_TIMER_HPP
