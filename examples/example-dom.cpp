#include <iostream>

#include <qcon-dom.hpp>

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

int main()
{
    const char * qconStr{
R"({
    "Name": "Phil's Phillies",
    "Open Time": T08:30:00,
    "Close Time": T22:00:00,
    "Employees": [
        {
            "Name": "Phil",
            "Birthday": D1969-03-17,
            "Full Time": true,
            "Hourly Wage": 35.0
        },
        {
            "Name": "Ted",
            "Birthday": D1981-09-01,
            "Full Time": false,
            "Hourly Wage": 21.5
        },
        {
            "Name": "Sal",
            "Birthday": D1996-04-22,
            "Full Time": true,
            "Hourly Wage": 24.0
        }
    ],
    "Last Updated": D2023-03-01T19:16:49.8490041-08:00
})"};

    // Decode QCON
    std::optional<qcon::Value> rootVal{qcon::decode(qconStr)};

    // Check that the decode was successful
    abortIf(!rootVal);

    // Give everyone a raise
    qcon::Object * rootObj{rootVal->object()};
    abortIf(!rootObj);

    qcon::Array * employeesArr{rootObj->at("Employees").array()};
    abortIf(!employeesArr);

    for (qcon::Value & employeeVal : *employeesArr)
    {
        qcon::Object * employeeObj{employeeVal.object()};
        abortIf(!employeeObj);

        double * wage{employeeObj->at("Hourly Wage").floater()};
        abortIf(!wage);

        *wage *= 1.15;
    }

    // Update the timestamp
    qcon::Datetime * lastUpdated{rootObj->at("Last Updated").datetime()};
    abortIf(!lastUpdated);
    abortIf(!lastUpdated->fromTimepoint(std::chrono::system_clock::now(), qcon::TimezoneFormat::utcOffset));

    // Encode the updated QCON
    std::optional<std::string> newQsonStr{qcon::encode(*rootVal)};
    abortIf(!newQsonStr);

    std::cout << *newQsonStr << std::endl;

    return 0;
}
