/**
 * Compile-time lib
 * @file    ct-string.hpp
 * @brief   A compile-time string structure for meta-programs
 * @author  Douglas Oliveira (
 * @date    2020-10-01
 */

// credits for the solution:
// https://stackoverflow.com/questions/15858141/conveniently-declaring-compile-time-strings-in-c#answer-15912824

#ifndef CT_STRING_HPP
#define CT_STRING_HPP

// Call this to create a compile-time string from a literal
#define CTSTRING(string_literal)                                                       \
    []{                                                                                \
        struct constexpr_string_type { const char * data = string_literal; };         \
        return ct::toolbox::apply_range<sizeof(string_literal)-1,                 \
            ct::toolbox::string_builder<constexpr_string_type>::produce>::result{};   \
    }()

namespace ct
{

template<char... str>
struct string
{
    static constexpr const char data[sizeof...(str)+1] = {str..., '\0'};
};

// concat operator
template<char... str0, char... str1>
constexpr string<str0..., str1...> operator+(string<str0...>, string<str1...>)
{
    return {};
}

// print operator
template<char... str>
std::ostream& operator << (std::ostream& out, string<str...> s) {
    return (out << s.data);
}

template<char... str>
constexpr const char string<str...>::data[sizeof...(str)+1];

namespace toolbox
{
    template<unsigned count, template<unsigned...> class meta_functor, unsigned... indices>
    struct apply_range
    {
        typedef typename apply_range<count-1, meta_functor, count-1, indices...>::result result;
    };

    template<template<unsigned...> class meta_functor, unsigned... indices>
    struct apply_range<0, meta_functor, indices...>
    {
        typedef typename meta_functor<indices...>::result result;
    };

    template<typename lambda_str_type>
    struct string_builder
    {
        template<unsigned... indices>
        struct produce
        {
            typedef string<lambda_str_type{}.data[indices]...> result;
        };
    };
}

}

#endif // CT_STRING_HPP
