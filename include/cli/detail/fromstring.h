/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016-2021 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef CLI_DETAIL_FROMSTRING_H_
#define CLI_DETAIL_FROMSTRING_H_

// #define CLI_FROMSTRING_USE_BOOST

#ifdef CLI_FROMSTRING_USE_BOOST

#include <boost/lexical_cast.hpp>

namespace cli
{
namespace detail
{

template <typename T>
inline
T from_string(const std::string& s)
{
    return boost::lexical_cast<T>(s);
}

} // namespace detail
} // namespace cli

#else

#include <exception>
#include <string>
#include <sstream>

namespace cli
{

    namespace detail
    {
        class bad_conversion : public std::bad_cast
        {
            public:
                const char* what() const noexcept override {
                    return "bad from_string conversion: "
                        "source string value could not be interpreted as target";
                }
        };

template <typename T>
inline T from_string(const std::string& s);

template <>
inline std::string from_string(const std::string& s)
{
    return s;
}

template <>
inline std::nullptr_t from_string(const std::string& /*s*/)
{
    return nullptr;
}

namespace detail
{

template <typename T>
inline T unsigned_digits_from_string(const std::string& s)
{
    if (s.empty())
        throw bad_conversion();
    T result = 0;
    for (char c: s)
    {
        if (!std::isdigit(c))
            throw bad_conversion();
        const T digit = static_cast<T>( c - '0' );
        const T tmp = (result * 10) + digit;
        if (result != ((tmp-digit)/10) || (tmp < result))
            throw bad_conversion();
        result = tmp;
    }
    return result;
}

template <typename T>
inline T unsigned_from_string(std::string s)
{
    if (s.empty())
        throw bad_conversion();
    if (s[0] == '+')
    {
        s = s.substr(1);
    }
    return unsigned_digits_from_string<T>(s);
}

template <typename T>
inline T signed_from_string(std::string s)
{
    if (s.empty())
        throw bad_conversion();
    using U = std::make_unsigned_t<T>;
    if (s[0] == '-')
    {
        s = s.substr(1);
        const U val = unsigned_digits_from_string<U>(s);
        if ( val > static_cast<U>( - std::numeric_limits<T>::min() ) )
            throw bad_conversion();
        return (- static_cast<T>(val));
    }
    else if (s[0] == '+')
    {
        s = s.substr(1);
    }
    const U val = unsigned_digits_from_string<U>(s);
    if (val > static_cast<U>( std::numeric_limits<T>::max() ))
        throw bad_conversion();
    return static_cast<T>(val);
}

} // detail

// signed

template <> inline signed char 
from_string(const std::string& s) { return detail::signed_from_string<signed char>(s); }

template <> inline short int 
from_string(const std::string& s) { return detail::signed_from_string<short int>(s); }

template <> inline int
from_string(const std::string& s) { return detail::signed_from_string<int>(s); }

template <> inline long int
from_string(const std::string& s) { return detail::signed_from_string<long int>(s); }

template <> inline long long int
from_string(const std::string& s) { return detail::signed_from_string<long long int>(s); }

// unsigned

template <> inline unsigned char
from_string(const std::string& s) { return detail::unsigned_from_string<unsigned char>(s); }

template <> inline unsigned short int
from_string(const std::string& s) { return detail::unsigned_from_string<unsigned short int>(s); }

template <> inline unsigned int
from_string(const std::string& s) { return detail::unsigned_from_string<unsigned int>(s); }

template <> inline unsigned long int
from_string(const std::string& s) { return detail::unsigned_from_string<unsigned long int>(s); }

template <> inline unsigned long long int
from_string(const std::string& s) { return detail::unsigned_from_string<unsigned long long int>(s); }

// bool

template <>
inline bool from_string(const std::string& s)
{
    if (s == "true") return true;
    else if (s == "false") return false;
    const auto value = detail::signed_from_string<long long int>(s);
    if (value == 1) return true;
    else if (value == 0) return false;
    throw bad_conversion();            
}

// chars

template <>
inline char from_string(const std::string& s)
{
    if (s.size() != 1) throw bad_conversion();
    return s[0];            
}

// floating points

template <>
inline float from_string(const std::string& s)
{
    if ( std::any_of(s.begin(), s.end(), [](char c){return std::isspace(c);} ) )
        throw bad_conversion();
    std::string::size_type sz;
    float result = {};
    try {
        result = std::stof(s, &sz);
    } catch (const std::exception&) {
        throw bad_conversion();
    }
    if (sz != s.size())
        throw bad_conversion();
    return result;
}

template <>
inline double from_string(const std::string& s)
{
    if ( std::any_of(s.begin(), s.end(), [](char c){return std::isspace(c);} ) )
        throw bad_conversion();
    std::string::size_type sz;
    double result = {};
    try {
        result = std::stod(s, &sz);
    } catch (const std::exception&) {
        throw bad_conversion();
    }
    if (sz != s.size())
        throw bad_conversion();
    return result;
}

template <>
inline long double from_string(const std::string& s)
{
    if ( std::any_of(s.begin(), s.end(), [](char c){return std::isspace(c);} ) )
        throw bad_conversion();
    std::string::size_type sz;
    long double result = {};
    try {
        result = std::stold(s, &sz);
    } catch (const std::exception&) {
        throw bad_conversion();
    }
    if (sz != s.size())
        throw bad_conversion();
    return result;
}

// fallback: operator <<

template <typename T>
inline T from_string(const std::string& s)
{
    std::stringstream interpreter;
    T result;

    if(!(interpreter << s) ||
        !(interpreter >> result) ||
        !(interpreter >> std::ws).eof())
        throw bad_conversion();

    return result;
}

    } // detail

} // cli


#endif // CLI_FROMSTRING_USE_BOOST

#endif // CLI_DETAIL_FROMSTRING_H_
