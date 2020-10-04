#ifdef CT_BASE_64_HPP

namespace impl
{
    static constexpr const b64char dict[64] = {
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
        '0','1','2','3','4','5','6','7','8','9','@','&'
    };

    static constexpr index_type indexOf(b64char c) {
        return index_type (
            ('A' <= c && c <= 'Z') ? (00 + c - 'A') :
            ('a' <= c && c <= 'z') ? (26 + c - 'a') :
            ('0' <= c && c <= '9') ? (52 + c - '0') :
            (c == '@') ? 62 : (c == '&') ? 63 : 64
        );
    }

    namespace checks {
        template<char... chars>
        constexpr auto is_base64_chars_v = std::integral_constant<bool, is_base64_chars_v<chars...> >::value;
        template<char head, char... tail>
        constexpr auto is_base64_chars_v<head, tail...> = std::integral_constant<bool, indexOf(head) < 64 && is_base64_chars_v<tail...> >::value;
        template<char head>
        constexpr auto is_base64_chars_v<head, '=', '='> = std::integral_constant<bool, indexOf(head) < 64 >::value;
        template<char head>
        constexpr auto is_base64_chars_v<head, '='> = std::integral_constant<bool, indexOf(head) < 64 >::value;
        template<>
        constexpr auto is_base64_chars_v<> = std::true_type::value;

        template<size_t n>
        constexpr auto is_base64_valid_length_v = std::integral_constant<bool, n % 4 == 0>::value;

        template<char... chars>
        constexpr auto is_base64_encoding_v = std::integral_constant<bool,
            is_base64_valid_length_v<sizeof...(chars)> &&
            is_base64_chars_v<chars...> >::value;
    }

    //////////// Encoder Impl. ////////////

    template<char...chars>
    struct CTBase64Encoder {
        static constexpr auto encoded_string = CTBase64Encoder<chars...>::encoded_string;
    };

    template<char c1, char c2, char c3, char...chars>
    struct CTBase64Encoder<c1, c2, c3, chars...> {
        typedef ct::string<dict[(c1  & 0xFC) >> 2],
                            dict[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)],
                            dict[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)],
                            dict[(c3  & 0x3F)]> value_type;

        static constexpr auto encoded_string = value_type() + CTBase64Encoder<chars...>::encoded_string;
    };
    template<char c1, char c2>
    struct CTBase64Encoder<c1, c2> {
        typedef ct::string<dict[(c1  & 0xFC) >> 2],
                            dict[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)],
                            dict[(c2  & 0x0F) << 2], '='> value_type;

        static constexpr auto encoded_string = value_type();
    };

    template<char c1>
    struct CTBase64Encoder<c1> {
        typedef ct::string<dict[(c1 & 0xFC) >> 2],
                            dict[(c1 & 0x03) << 4], '=','='> value_type;

        static constexpr auto encoded_string = value_type();
    };

    template<>
    struct CTBase64Encoder<> {
        static constexpr auto encoded_string = ct::string<>();
    };

    //////////// Decoder Impl. ////////////

    template<int32_t s>
    using string_from_int = ct::string<(s >> 16) & 0xFF, (s >> 8) & 0xFF, s & 0xFF>;

    template<b64char... chars>
    struct b64_decoder_impl {
        static constexpr auto decoded_string = b64_decoder_impl<chars...>::decoded_string;
    };

    template<b64char c1, b64char c2, b64char c3, b64char c4, b64char... chars>
    struct b64_decoder_impl<c1, c2, c3, c4, chars...> {
        typedef string_from_int<indexOf(c1) << 18 | indexOf(c2) << 12 |
                                indexOf(c3) <<  6 | indexOf(c4)> value_type;

        static constexpr auto decoded_string = value_type() + b64_decoder_impl<chars...>::decoded_string;
    };

    template<b64char c1, b64char c2, b64char c3>
    struct b64_decoder_impl<c1, c2, c3, '='> {
        typedef string_from_int<indexOf(c1) << 18 | indexOf(c2) << 12 | indexOf(c3) << 6> value_type;

        static constexpr auto decoded_string = value_type();
    };

    template<b64char c1, b64char c2>
    struct b64_decoder_impl<c1, c2, '=', '='> {
        typedef string_from_int<indexOf(c1) << 18 | indexOf(c2) << 12> value_type;

        static constexpr auto decoded_string = value_type();
    };

    template<>
    struct b64_decoder_impl<> {
        static constexpr auto decoded_string = ct::string<>();
    };

    template<bool isvalid, b64char... chars>
    struct b64_decode_if {
        static_assert(isvalid, "Input string is not a valid base 64 encoding");
        static constexpr auto decoded_string = ct::string<>();
    };

    template<b64char... chars>
    struct b64_decode_if<true, chars...> {
        static constexpr auto decoded_string = b64_decoder_impl<chars...>::decoded_string;
    };

    template<b64char... chars>
    struct CTBase64Decoder : b64_decode_if<checks::is_base64_encoding_v<chars...>, chars...> { };
}

#endif // CT_BASE_64_HPP
