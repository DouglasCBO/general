/**
 * Compile-time lib
 * @file    ct-base64.hpp
 * @brief   A compile-time Base64 encoder/decoder
 * @author  Douglas Oliveira
 * @date    2020-10-01
 *
 * @note !!C++14 dependent module!!
 */

#ifndef CT_BASE_64_HPP
#define CT_BASE_64_HPP

#include "ct-string.hpp"

// macro helpers to encode/decode string literals at compile-time
#define CT_BASE64_ENCODE(string_literal) \
    ct::Base64::encode(CTSTRING(string_literal))
#define CT_BASE64_DECODE(string_b64_literal) \
    ct::Base64::decode(CTSTRING(string_b64_literal))
// the versions below can be assigned to a std::string at runtime
#define CT_BASE64_ENCODE_RT(string_literal) \
    CT_BASE64_ENCODE(string_literal).data
#define CT_BASE64_DECODE_RT(string_literal) \
    CT_BASE64_DECODE(string_literal).data

namespace ct
{

typedef char b64char;
typedef uint8_t index_type;

struct Base64 {
    // return a compile-time string
    template <char... str>
    static constexpr auto encode(ct::string<str...> s);
    // return a compile-time string
    template <char... str>
    static constexpr auto decode(ct::string<str...> s);
};

#include "ct-base64-impl.h"

template <char... str>
constexpr auto Base64::encode(ct::string<str...> s) {
    return impl::CTBase64Encoder<str...>::encoded_string;
}

template <b64char... str>
constexpr auto Base64::decode(ct::string<str...> s) {
    return impl::CTBase64Decoder<str...>::decoded_string;
}

}

#endif // CT_BASE_64_HPP
