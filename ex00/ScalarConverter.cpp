#include "ScalarConverter.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <limits>
#include <climits>
#include <cctype>
#include <iomanip>
#include <cerrno>
#include <cstdlib>


static bool isChar(const std::string& input)
{
    if (input.length() == 3 && input[0] == '\'' && input[2] == '\'')
    {
        unsigned char c = static_cast<unsigned char>(input[1]);
        return std::isprint(c);
    }
    return false;
}

static bool isInt(const std::string& input)
{
    size_t start = 0;
    size_t len = input.length();

    if (input.empty())
        return false;
    
    if (input[0] == '+' || input[0] == '-')
        start++;

    if (len == 1 && start == 1)
        return false;

    for (size_t i = start; i < len; i++)
    {
        if (!std::isdigit(static_cast<int>(input[i])))
            return false;
    }
    
    return true;

    
}

static bool isFloat(const std::string& input)
{

    if (input == "nanf" || input == "+inff" || input == "-inff" || input == "inff")
        return true;

    if (input.length() < 3 || input[input.length() - 1] != 'f')
        return false;
    
    std::string trimmed_str = input.substr(0, input.length() - 1);
    bool dot = false;
    bool digit = false;

    size_t start = 0;

    if (trimmed_str[start] == '+' || trimmed_str[start] == '-')
        start++;

    for (size_t i = start; i < trimmed_str.size(); i++)
    {
        if (trimmed_str[i] == '.')
        {
            if (dot == true)
                return false;
            dot = true;
        }
        else if (std::isdigit(static_cast<unsigned char>(trimmed_str[i])))
        {
            digit = true;
        }
        else
        {
            return false;
        }
    }

    return dot && digit;


}

static bool isDouble(const std::string& input)
{
    if (input == "nan" || input == "+inf" || input == "-inf" || input == "inf")
        return true;

    if (input.length() < 2)
        return false;

    bool dot = false;
    bool digit = false;
    size_t start = 0;

    if (input[0] == '+' || input[0] == '-')
        start++;

    for (size_t i = start; i < input.size(); i++)
    {
        if (input[i] == '.')
        {
            if (dot)
                return false;
            dot = true;
        }
        else if (std::isdigit(static_cast<unsigned char>(input[i])))
        {
            digit = true;
        }
        else
        {
            return false;
        }
    }

    return dot && digit;
}


static char    parseChar(const std::string& input)
{
    return input[1];
}

static double parseIntLiteral(const std::string& input)
{
    char* end;
    errno = 0;
    double value = std::strtod(input.c_str(), &end);

    if (*end != '\0')
        throw std::invalid_argument("Not an Integer");

    return value;
}


static float parseFloat(const std::string& input)
{
    if (input == "nanf")
        return std::numeric_limits<float>::quiet_NaN();
    if (input == "+inff" || input == "inff")
        return std::numeric_limits<float>::infinity();
    if (input == "-inff")
        return -std::numeric_limits<float>::infinity();

    std::string trimmed = input.substr(0, input.length() - 1);
    char* end;
    errno = 0;
    double value = std::strtod(trimmed.c_str(), &end);

    if (*end != '\0')
        throw std::invalid_argument("Not a Float");

    return static_cast<float>(value);
}

static double parseDouble(const std::string& input)
{
    if (input == "nan")
        return std::numeric_limits<double>::quiet_NaN();
    if (input == "+inf" || input == "inf")
        return std::numeric_limits<double>::infinity();
    if (input == "-inf")
        return -std::numeric_limits<double>::infinity();

    char* end;
    errno = 0;
    double value = std::strtod(input.c_str(), &end);

    if (*end != '\0')
        throw std::invalid_argument("Not a Double");

    return value;
}


static std::string formatNumber(double value, int precision)
{
    std::ostringstream oss;
    oss << std::setprecision(precision) << value;
    std::string s = oss.str();

    if (s.find_first_of(".eE") == std::string::npos)
        s += ".0";

    return s;
}

static void printChar(double value)
{
    std::cout << "char: ";

    if (value != value || value < 0 || value > 127)
        std::cout << "impossible\n";
    else if (!std::isprint(static_cast<unsigned char>(value)))
        std::cout << "Non displayable\n";
    else
        std::cout << '\'' << static_cast<char>(value) << "'\n";
}

static void printInt(double value)
{
    std::cout << "int: ";

    if (value != value || value < INT_MIN || value > INT_MAX)
        std::cout << "impossible\n";
    else
        std::cout << static_cast<int>(value) << '\n';
}

static void printFloat(double value)
{
    std::cout << "float: ";

    if (value != value)
        std::cout << "nanf\n";
    else if (value == std::numeric_limits<double>::infinity())
        std::cout << "inff\n";
    else if (value == -std::numeric_limits<double>::infinity())
        std::cout << "-inff\n";
    else
        std::cout << formatNumber(static_cast<float>(value), std::numeric_limits<float>::digits10) << 'f' << '\n';
}

static void printDouble(double value)
{
    std::cout << "double: ";

    if (value != value)
        std::cout << "nan\n";
    else if (value == std::numeric_limits<double>::infinity())
        std::cout << "inf\n";
    else if (value == -std::numeric_limits<double>::infinity())
        std::cout << "-inf\n";
    else
        std::cout << formatNumber(value, std::numeric_limits<double>::digits10) << '\n';
}

void ScalarConverter::convert(const std::string& input)
{
    try
    {
        if (isChar(input))
        {
            double value = static_cast<unsigned char>(parseChar(input));
            printChar(value);
            printInt(value);
            printFloat(value);
            printDouble(value);
        }
        else if (isInt(input))
        {
            double value = parseIntLiteral(input);
            printChar(value);
            printInt(value);
            printFloat(value);
            printDouble(value);
        }
        else if (isFloat(input))
        {
            double value = static_cast<double>(parseFloat(input));
            printChar(value);
            printInt(value);
            printFloat(value);
            printDouble(value);
        }
        else if (isDouble(input))
        {
            double value = parseDouble(input);
            printChar(value);
            printInt(value);
            printFloat(value);
            printDouble(value);
        }
        else
        {
            std::cout << "Invalid input\n";
        }
    }
    catch (const std::exception&)
    {
        std::cout << "Invalid input\n";
    }
}

ScalarConverter::ScalarConverter() {}
ScalarConverter::ScalarConverter(const ScalarConverter&) {}
ScalarConverter& ScalarConverter::operator=(const ScalarConverter&) { return *this; }
ScalarConverter::~ScalarConverter() {}