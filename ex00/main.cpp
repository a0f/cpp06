#include "ScalarConverter.hpp"
#include <iostream>

static void testConvert(const std::string& label, const std::string& input)
{
    std::cout << label << ": " << input << std::endl;
    ScalarConverter::convert(input);
    std::cout << '\n';
}

static void runTests(void)
{
    std::cout << "------------------------------------\n";
    std::cout << "Char literal tests\n";
    std::cout << "------------------------------------\n";
    testConvert("printable char",       "'a'");
    testConvert("printable char space", "' '");
    testConvert("printable char star",  "'*'");
    testConvert("digit as char input",  "'5'");

    std::cout << "------------------------------------\n";
    std::cout << "Int tests\n";
    std::cout << "------------------------------------\n";
    testConvert("zero",        "0");
    testConvert("positive",    "42");
    testConvert("negative",    "-42");
    testConvert("int max",     "2147483647");
    testConvert("int min",     "-2147483648");
    testConvert("int overflow","2147483648");
    testConvert("non-display", "31");
    testConvert("char range",  "65");

    std::cout << "------------------------------------\n";
    std::cout << "Float tests\n";
    std::cout << "------------------------------------\n";
    testConvert("zero float",     "0.0f");
    testConvert("positive float", "42.0f");
    testConvert("negative float", "-4.2f");
    testConvert("precision",      "0.1f");
    testConvert("large float",    "1234567.0f");
    testConvert("+inff",          "+inff");
    testConvert("-inff",          "-inff");
    testConvert("nanf",           "nanf");

    std::cout << "------------------------------------\n";
    std::cout << "Double tests\n";
    std::cout << "------------------------------------\n";
    testConvert("zero double",     "0.0");
    testConvert("positive double", "42.0");
    testConvert("negative double", "-4.2");
    testConvert("precision",       "0.1");
    testConvert("+inf",            "+inf");
    testConvert("-inf",            "-inf");
    testConvert("nan",             "nan");

    std::cout << "------------------------------------\n";
    std::cout << "Subject examples\n";
    std::cout << "------------------------------------\n";
    testConvert("subject: 0",     "0");
    testConvert("subject: nan",   "nan");
    testConvert("subject: 42.0f", "42.0f");

    std::cout << "------------------------------------\n";
    std::cout << "Invalid / edge cases\n";
    std::cout << "------------------------------------\n";
    testConvert("empty string",        "");
    testConvert("word",                "abc");
    testConvert("trailing chars",      "42abc");
    testConvert("double dot",          "1.2.3");
    testConvert("sign only",           "-");
    testConvert("f only",              "f");
    testConvert("no quotes bare char", "a");
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        ScalarConverter::convert(argv[1]);
        return 0;
    }

    runTests();
    return 0;
}
