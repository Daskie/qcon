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

    enum class TimezoneFormat : u8
    {
        localTime, /// The next datetime will be given no timezone specifier
        utc,       /// The next datetime will be given the `Z` specifier
        utcOffset, /// The next datetime will be given an offset specifier, e.g. `+03:00`
    };
    using enum TimezoneFormat;

    struct Date
    {
        u16 year{1970u}; /// Must be in range [0, 9999]
        u8 month{1u};    /// Must be in range [1, 12]
        u8 day{1u};      /// Must be in range [1, 31]

        ///
        /// Compare dates chronologically
        ///
        [[nodiscard]] auto operator<=>(const Date &) const = default;
    };

    struct Time
    {
        u8 hour{};       /// Must be in range [0, 23]
        u8 minute{};     /// Must be in range [0, 59]
        u8 second{};     /// Must be in range [0, 59]
        u32 subsecond{}; /// Nanoseconds; must be in range [0, 999,999,999]

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

    inline bool Datetime::fromTimepoint(const Timepoint timepoint, const TimezoneFormat timezoneFormat)
    {
        std::chrono::system_clock::duration duration{timepoint.time_since_epoch()};
        std::chrono::minutes timezoneOffset{};

        // Determine timezone offset
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

        // Determine YMD
        const std::chrono::days days{std::chrono::floor<std::chrono::days>(duration)};
        const std::chrono::year_month_day ymd{std::chrono::sys_days{days}};

        // Verify year is four digits
        if (ymd.year() < std::chrono::year{0} || ymd.year() > std::chrono::year{9999})
        {
            return false;
        }

        // Decompose remaining duration into nanoseconds, seconds, minutes, and hours
        u64 nanoseconds{u64(std::chrono::nanoseconds(duration - days).count())};
        u64 seconds{nanoseconds / 1'000'000'000u}; nanoseconds %= 1'000'000'000u;
        u64 minutes{seconds / 60u}; seconds %= 60u;
        u64 hours{minutes / 60u}; minutes %= 60u;

        date.year = u16(static_cast<int>(ymd.year()));
        date.month = u8(static_cast<unsigned int>(ymd.month()));
        date.day = u8(static_cast<unsigned int>(ymd.day()));
        time.hour = u8(hours);
        time.minute = u8(minutes);
        time.second = u8(seconds);
        time.subsecond = u32(nanoseconds);
        zone.format = timezoneFormat;
        zone.offset = s16(timezoneOffset.count());
        return true;
    }

    inline Timepoint Datetime::toTimepoint() const
    {
        const std::chrono::year_month_day ymd{std::chrono::year{date.year}, std::chrono::month{date.month}, std::chrono::day{date.day}};
        std::chrono::system_clock::duration duration{std::chrono::sys_days{ymd}.time_since_epoch()};
        duration += std::chrono::hours{time.hour};
        duration += std::chrono::minutes{time.minute};
        duration += std::chrono::seconds{time.second};
        duration += std::chrono::round<std::chrono::system_clock::duration>(std::chrono::nanoseconds{time.subsecond});

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
