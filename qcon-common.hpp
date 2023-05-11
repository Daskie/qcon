#pragma once

#include <cstdint>

#include <chrono>
#include <compare>
#include <string>
#include <string_view>

namespace qcon
{
    using  s8 =   int8_t;
    using s16 =  int16_t;
    using s32 =  int32_t;
    using s64 =  int64_t;
    using  u8 =  uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using unat = size_t;

    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using Timepoint = std::chrono::system_clock::time_point;

    enum class Container
    {
        end,
        object,
        array
    };
    using enum Container;

    enum class Density
    {
        multiline, /// Elements are put on new lines
        uniline,   /// Elements are put on one line separated by spaces
        nospace    /// No space is used whatsoever
    };
    using enum Density;

    enum class Base
    {
        decimal,
        binary,
        octal,
        hex
    };
    using enum Base;

    enum class TimezoneFormat : u8
    {
        localTime, /// The next datetime will be given no timezone specifier
        utc,       /// The next datetime will be given the `Z` specifier
        utcOffset, /// The next datetime will be given an offset specifier, e.g. `+03:00`
    };
    using enum TimezoneFormat;

    struct Date
    {
        /// Convenience wrapper for `fromYmd`
        [[nodiscard]] static Date from(const std::chrono::year_month_day & ymd);

        u16 year{1970u}; /// Must be in range [0, 9999]
        u8 month{1u};    /// Must be in range [1, 12]
        u8 day{1u};      /// Must be in range [1, 31]

        ///
        /// @param ymd `std::chrono::year_month_day` to convert from; all fields must satisfy range requirements above
        ///
        void fromYmd(const std::chrono::year_month_day & ymd);

        ///
        /// @return date converted to a `std::chrono::year_month_day`
        ///
        [[nodiscard]] std::chrono::year_month_day toYmd() const;

        ///
        /// Compare dates chronologically
        ///
        [[nodiscard]] auto operator<=>(const Date &) const = default;
    };

    struct Time
    {
        /// Convenience wrapper for `fromDuration`
        [[nodiscard]] static Time from(std::chrono::nanoseconds ns);

        u8 hour{};       /// Must be in range [0, 23]
        u8 minute{};     /// Must be in range [0, 59]
        u8 second{};     /// Must be in range [0, 59]
        u32 subsecond{}; /// Nanoseconds; must be in range [0, 999,999,999]

        ///
        /// Convert from a `std::chrono` duration
        /// @param ns nanoseconds since midnight; must be positive and less than the number of nanoseconds in a day
        ///
        void fromDuration(std::chrono::nanoseconds ns);

        ///
        /// Convert to a `std::chrono` duration
        /// @return nanoseconds since midnight
        ///
        [[nodiscard]] std::chrono::nanoseconds toDuration() const;

        ///
        /// Compare times chronologically
        ///
        [[nodiscard]] auto operator<=>(const Time &) const = default;

        /// Ensure we're not loosing system timepoint precision by storing only nanoseconds
        static_assert(std::chrono::system_clock::duration::period::den <= 1'000'000'000);
    };

    struct Timezone
    {
        TimezoneFormat format{};
        s16 offset{}; /// Minutes; must be in range [-1439, 1439]
    };

    struct Datetime
    {
        /// Convenience wrapper for `fromTimepoint`
        [[nodiscard]] static std::pair<Datetime, bool> from(Timepoint timepoint, TimezoneFormat timezoneFormat);

        Date date{};
        Time time{};
        Timezone zone{};

        ///
        /// Convert from a system timepoint
        /// @param timepoint system timepoint to convert
        /// @param timezoneFormat timezone format to use for our zone
        /// @return whether the timepoint could be represented as a datetime / the conversion was successful
        ///
        [[nodiscard]] bool fromTimepoint(Timepoint timepoint, TimezoneFormat timezoneFormat);

        ///
        /// Convert a datetime to a system timepoint
        /// @return system timepoint corresponding to this datetime
        ///
        [[nodiscard]] Timepoint toTimepoint() const;
    };

    static_assert(sizeof(Date) == 4u);
    static_assert(sizeof(Time) == 8u);
    static_assert(sizeof(Datetime) == 16u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    inline constexpr bool _isDigit(const char c)
    {
        return unat(c - '0') < 10u;
    }

    inline constexpr bool _isControl(const char c)
    {
        return u8(c) < 32u;
    }

    inline Date Date::from(const std::chrono::year_month_day & ymd)
    {
        Date date;
        date.fromYmd(ymd);
        return date;
    }

    inline void Date::fromYmd(const std::chrono::year_month_day & ymd)
    {
        year = u16(static_cast<int>(ymd.year()));
        month = u8(static_cast<unsigned int>(ymd.month()));
        day = u8(static_cast<unsigned int>(ymd.day()));
    }

    inline std::chrono::year_month_day Date::toYmd() const
    {
        return std::chrono::year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}};
    }

    inline Time Time::from(const std::chrono::nanoseconds ns)
    {
        Time time;
        time.fromDuration(ns);
        return time;
    }

    inline void Time::fromDuration(const std::chrono::nanoseconds ns)
    {
        static constexpr u64 nsPerSecond{1'000'000'000u};
        static constexpr u64 nsPerMinute{nsPerSecond * 60u};
        static constexpr u64 nsPerHour{nsPerMinute * 60u};

        u64 nanoseconds{u64(ns.count())};
        hour   = u8(nanoseconds /   nsPerHour); nanoseconds %=   nsPerHour;
        minute = u8(nanoseconds / nsPerMinute); nanoseconds %= nsPerMinute;
        second = u8(nanoseconds / nsPerSecond); nanoseconds %= nsPerSecond;
        subsecond = u32(nanoseconds);
    }

    inline std::chrono::nanoseconds Time::toDuration() const
    {
        // Order is important to avoid uneccessary intermediate multiplications
        return std::chrono::nanoseconds{subsecond} + std::chrono::seconds{second} + std::chrono::minutes{minute} + std::chrono::hours{hour};
    }

    inline std::pair<Datetime, bool> Datetime::from(const Timepoint timepoint, const TimezoneFormat timezoneFormat)
    {
        std::pair<Datetime, bool> res;
        res.second = res.first.fromTimepoint(timepoint, timezoneFormat);
        return res;
    }

    inline bool Datetime::fromTimepoint(const Timepoint timepoint, const TimezoneFormat timezoneFormat)
    {
        std::chrono::system_clock::duration duration{timepoint.time_since_epoch()};
        std::chrono::minutes timezoneOffset{};

        // Timezone
        {
            if (timezoneFormat != utc)
            {
                const std::chrono::time_zone * const timeZone{std::chrono::current_zone()};
                timezoneOffset = std::chrono::round<std::chrono::minutes>(timeZone->get_info(timepoint).offset);

                // Verify offset is no more than two hour and two minute digits worth
                if (std::chrono::abs(timezoneOffset) >= std::chrono::minutes{100 * 60})
                {
                    return false;
                }

                duration += timezoneOffset;
            }

            zone.format = timezoneFormat;
            zone.offset = s16(timezoneOffset.count());
        }

        // Date
        const std::chrono::days days{std::chrono::floor<std::chrono::days>(duration)};
        {
            const std::chrono::year_month_day ymd{std::chrono::sys_days{days}};

            if (ymd.year() < std::chrono::year{0} || ymd.year() > std::chrono::year{9999})
            {
                return false;
            }

            date.fromYmd(ymd);
        }

        // Time
        // Decompose remaining duration into nanoseconds, seconds, minutes, and hours
        time.fromDuration(duration - days);

        return true;
    }

    inline Timepoint Datetime::toTimepoint() const
    {
        const std::chrono::year_month_day ymd{date.toYmd()};
        std::chrono::system_clock::duration duration{std::chrono::sys_days{ymd}.time_since_epoch()};
        duration += std::chrono::round<std::chrono::system_clock::duration>(time.toDuration());

        if (zone.format == localTime)
        {
            const std::chrono::local_info localInfo{std::chrono::current_zone()->get_info(std::chrono::local_time<std::chrono::system_clock::duration>{duration})};
            return Timepoint{duration - localInfo.first.offset};
        }
        else
        {
            return Timepoint{duration - std::chrono::minutes{zone.offset}};
        }
    }
}
