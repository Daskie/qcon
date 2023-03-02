#include <iostream>

#include <qcon-encode.hpp>
#include <qcon-decode.hpp>

struct Employee
{
    std::string name;
    std::chrono::year_month_day birthday;
    bool fullTime;
    double hourlyWage;

    bool operator==(const Employee &) const = default;
};

struct Business
{
    std::string name;
    std::chrono::minutes openTime;
    std::chrono::minutes closeTime;
    std::vector<Employee> employees;
    std::chrono::system_clock::time_point lastUpdated;

    bool operator==(const Business &) const = default;
};

void abortIf(const bool condition)
{
    if (condition)
    {
        std::abort();
    }
}

qcon::Encoder & operator<<(qcon::Encoder & encoder, const Employee & employee)
{
    encoder << qcon::object;
    encoder << "Name" << employee.name;
    encoder << "Birthday" << qcon::Date::from(employee.birthday);
    encoder << "Full Time" << employee.fullTime;
    encoder << "Hourly Wage" << employee.hourlyWage;
    encoder << qcon::end;

    return encoder;
}

qcon::Encoder & operator<<(qcon::Encoder & encoder, const Business & business)
{
    encoder << qcon::object;
    encoder << "Name" << business.name;
    encoder << "Open Time" << qcon::Time::from(business.openTime);
    encoder << "Close Time" << qcon::Time::from(business.closeTime);
    encoder << "Employees" << qcon::array;
    for (const Employee & employee: business.employees) encoder << employee;
    encoder << qcon::end;
    encoder << "Last Updated" << business.lastUpdated;
    encoder << qcon::end;

    return encoder;
}

std::string encodeExample(const Business & business)
{
    // Encode QCON
    qcon::Encoder encoder{};
    encoder << business;
    const std::optional<std::string> qconStr{encoder.finish()};

    // Check for encoding error
    if (!qconStr)
    {
        std::abort();
    }

    return *qconStr;
}

Employee decodeEmployee(qcon::Decoder & decoder)
{
    Employee employee;

    abortIf(!(decoder >> qcon::object));

    while (decoder.more())
    {
        abortIf(!(decoder >> decoder.key));

        if (decoder.key == "Name")
        {
            abortIf(!(decoder >> employee.name));
        }
        else if (decoder.key == "Birthday")
        {
            abortIf(!(decoder >> decoder.date));
            employee.birthday = decoder.date.toYmd();
        }
        else if (decoder.key == "Full Time")
        {
            abortIf(!(decoder >> employee.fullTime));
        }
        else if (decoder.key == "Hourly Wage")
        {
            abortIf(!(decoder >> employee.hourlyWage));
        }
    }

    return employee;
}

Business decodeBusiness(qcon::Decoder & decoder)
{
    Business business;

    abortIf(!(decoder >> qcon::object));

    while (decoder.more())
    {
        abortIf(!(decoder >> decoder.key));

        if (decoder.key == "Name")
        {
            abortIf(!(decoder >> business.name));
        }
        else if (decoder.key == "Open Time")
        {
            abortIf(!(decoder >> decoder.time));
            business.openTime = std::chrono::duration_cast<std::chrono::minutes>(decoder.time.toDuration());
        }
        else if (decoder.key == "Close Time")
        {
            abortIf(!(decoder >> decoder.time));
            business.closeTime = std::chrono::duration_cast<std::chrono::minutes>(decoder.time.toDuration());
        }
        else if (decoder.key == "Employees")
        {
            abortIf(!(decoder >> qcon::array));

            while (decoder.more())
            {
                business.employees.push_back(decodeEmployee(decoder));
            }
        }
        else if (decoder.key == "Last Updated")
        {
            abortIf(!(decoder >> decoder.datetime));
            business.lastUpdated = decoder.datetime.toTimepoint();
        }
    }

    return business;
}

Business decodeExample(const std::string & qconStr)
{
    qcon::Decoder decoder{qconStr};
    Business business{decodeBusiness(decoder)};
    abortIf(!decoder.finished());
    return business;
}

int main()
{
    // Create example business
    Business business;
    business.name = "Phil's Phillies";
    business.openTime = std::chrono::hours{8} + std::chrono::minutes{30};
    business.closeTime = std::chrono::hours{22};
    business.employees.push_back(Employee{
        .name = "Phil",
        .birthday = std::chrono::year_month_day{std::chrono::year{1969}, std::chrono::month{3}, std::chrono::day{17}},
        .fullTime = true,
        .hourlyWage = 35.0});
    business.employees.push_back(Employee{
        .name = "Ted",
        .birthday = std::chrono::year_month_day{std::chrono::year{1981}, std::chrono::month{9}, std::chrono::day{1}},
        .fullTime = false,
        .hourlyWage = 21.5});
    business.employees.push_back(Employee{
        .name = "Sal",
        .birthday = std::chrono::year_month_day{std::chrono::year{1996}, std::chrono::month{4}, std::chrono::day{22}},
        .fullTime = true,
        .hourlyWage = 24.0});
    business.lastUpdated = std::chrono::system_clock::now();

    // Encode QCON
    const std::string qconStr{encodeExample(business)};

    // Print encoded QCON
    std::cout << "Encoded:\n" << qconStr << std::endl;

    // Decode QCON
    const Business decodedBusiness{decodeExample(qconStr)};

    // Ensure decoded object matches original
    abortIf(decodedBusiness != business);

    return 0;
}
