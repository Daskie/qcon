#include <iostream>

#include <qcon-encode.hpp>

namespace
{
    void densityExample()
    {
        qcon::Encoder encoder{};
        encoder << qcon::object;
            encoder << "Multiline Object" << qcon::multiline << qcon::object << "k1" << 1 << "k2" << 2 << "k3" << 3 << qcon::end;
            encoder << "Uniline Object"   << qcon::uniline   << qcon::object << "k1" << 1 << "k2" << 2 << "k3" << 3 << qcon::end;
            encoder << "Nospace Object"   << qcon::nospace   << qcon::object << "k1" << 1 << "k2" << 2 << "k3" << 3 << qcon::end;
            encoder << "Multiline Array" << qcon::multiline << qcon::array << 1 << 2 << 3 << qcon::end;
            encoder << "Uniline Array"   << qcon::uniline   << qcon::array << 1 << 2 << 3 << qcon::end;
            encoder << "Nospace Array"   << qcon::nospace   << qcon::array << 1 << 2 << 3 << qcon::end;
        encoder << qcon::end;
        const std::optional<std::string> qconStr{encoder.finish()};

        // Check for encoding error
        if (!qconStr)
        {
            std::abort();
        }

        // Print results
        std::cout << "Density Example:\n\n";
        std::cout << *qconStr << '\n';
        std::cout << std::endl;
    }

    void integerBaseExample()
    {
        qcon::Encoder encoder{};
        encoder << qcon::object;
            encoder << "Decimal" << 123;
            encoder << "Hex" << qcon::hex << 123;
            encoder << "Octal" << qcon::octal << 123;
            encoder << "Binary" << qcon::binary << 123;
        encoder << qcon::end;
        const std::optional<std::string> qconStr{encoder.finish()};

        // Check for encoding error
        if (!qconStr)
        {
            std::abort();
        }

        // Print results
        std::cout << "Integer Base Example:\n\n";
        std::cout << *qconStr << '\n';
        std::cout << std::endl;
    }

    void timezoneFormatExample()
    {
        const std::chrono::system_clock::time_point timepoint{std::chrono::system_clock::now()};

        qcon::Encoder encoder{};
        encoder << qcon::object;
            encoder << "Local" << qcon::localTime << timepoint;
            encoder << "UTC" << qcon::utc << timepoint;
            encoder << "UTC Offset" << qcon::utcOffset << timepoint;
        encoder << qcon::end;
        const std::optional<std::string> qconStr{encoder.finish()};

        // Check for encoding error
        if (!qconStr)
        {
            std::abort();
        }

        // Print results
        std::cout << "Timezone Format Example:\n\n";
        std::cout << *qconStr << '\n';
        std::cout << std::endl;
    }

    void encoderOptionsExample()
    {
        const std::chrono::system_clock::time_point timepoint{std::chrono::system_clock::now()};

        // Can pass a base density to encoder constructor
        {
            qcon::Encoder encoder{qcon::uniline};
            encoder << qcon::array << "New" << "base" << "density" << qcon::end;
            const std::optional<std::string> qconStr{encoder.finish()};

            // Check for encoding error
            if (!qconStr)
            {
                std::abort();
            }

            // Print results
            std::cout << "Encoder Base Density Example:\n\n";
            std::cout << *qconStr << '\n';
            std::cout << std::endl;
        }

        // Can specify a custom indentation other than the default four spaces
        {
            // Use tabs for indentation
            qcon::Encoder encoder{qcon::multiline, "\t"};
            encoder << qcon::array;
                encoder << qcon::array;
                    encoder << 1;
                    encoder << 2;
                encoder << qcon::end;
                encoder << 3;
            encoder << qcon::end;
            const std::optional<std::string> qconStr{encoder.finish()};

            // Check for encoding error
            if (!qconStr)
            {
                std::abort();
            }

            // Print results
            std::cout << "Encoder Indentation Option Example:\n\n";
            std::cout << *qconStr << '\n';
            std::cout << std::endl;
        }
    }
}

int main()
{
    densityExample();
    integerBaseExample();
    timezoneFormatExample();
    encoderOptionsExample();

    return 0;
}
