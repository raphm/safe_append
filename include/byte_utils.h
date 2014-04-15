//
//  byte_utils.h
//  safe-append-cpp
//
//  Created by Raphael Martelles on 4/15/14.
//
//

#ifndef safe_append_cpp_byte_utils_h
#define safe_append_cpp_byte_utils_h

typedef unsigned char byte;

template<typename T_iter>
inline uint32_t extract_little_endian(T_iter & data) {
    static_assert(std::is_same<typename std::iterator_traits<T_iter>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "argument must be a random access iterator");
    return (data[0]<<0) | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);
}

template<typename T_iter>
inline uint32_t extract_big_endian(T_iter & data) {
    static_assert(std::is_same<typename std::iterator_traits<T_iter>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "argument must be a random access iterator");
    return (data[3]<<0) | (data[2]<<8) | (data[1]<<16) | (data[0]<<24);
}

template<typename T_iter>
inline void encode_little_endian(T_iter const & data, uint32_t value) {
    static_assert(std::is_same<typename std::iterator_traits<T_iter>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "argument must be a random access iterator");
    data[3]=(byte)((0xFF000000&value)>>24);
    data[2]=(byte)((0x00FF0000&value)>>16);
    data[1]=(byte)((0x0000FF00&value)>>8);
    data[0]=(byte)((0x000000FF&value)>>0);
}

template<typename T_iter>
inline void encode_big_endian(T_iter const & data, uint32_t value) {
    static_assert(std::is_same<typename std::iterator_traits<T_iter>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "argument must be a random access iterator");
    data[0]=(byte)((0xFF000000&value)>>24);
    data[1]=(byte)((0x00FF0000&value)>>16);
    data[2]=(byte)((0x0000FF00&value)>>8);
    data[3]=(byte)((0x000000FF&value)>>0);
}

inline char nibble_to_hex(byte b) {
    b&=0x0F;
    return (b<10) ? ('0'+b) : ('a'+(b-10));
}

inline char hinibble(byte b) { return nibble_to_hex(b>>4); }
inline char lonibble(byte b) { return nibble_to_hex(b); }

template<typename T_iter>
inline void bytes_to_hex(T_iter const & bytes_begin,
                         T_iter const & bytes_end,
                         std::back_insert_iterator<std::string> & bi_str) {
    static_assert(std::is_same<typename std::iterator_traits<T_iter>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "Arguments bytes_begin and bytes_end must be random access iterators.");
    for(T_iter it=bytes_begin; it<bytes_end; ++it) {
        *bi_str=hinibble(*it);
        *bi_str=lonibble(*it);
    }
}

inline byte hex_to_nibble(char c) {
    if('0' <= c && c <= '9') {
        return (c-'0');
    } else if('a' <= c && c <= 'f') {
        return (c-'a')+10;
    } else if('A' <= c && c <= 'F') {
        return (c-'A')+10;
    } else {
        return 0;
    }
}

template<typename T>
inline void hex_to_bytes(std::string const & str, std::back_insert_iterator<std::vector<byte>> & bi_bytevec) {
    for(int i=0;i<str.length();i+=2) {
        byte b = hex_to_nibble(str[i+1]);
        b |= (0xF0 & (hex_to_nibble(str[i]))<<4);
        *bi_bytevec = b;
    }
}

inline std::vector<byte> hex_to_bytes(std::string const & str) {
    std::vector<byte> bv;
    std::back_insert_iterator<std::vector<byte>> bvi = std::back_inserter(bv);
    hex_to_bytes<byte>(str, bvi);
    return bv;
}

template<typename T_cont, typename U_cont>
inline bool bytes_equal(T_cont const & b1, U_cont const & b2) {
    static_assert(std::is_same<typename std::iterator_traits<typename T_cont::iterator>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "Argument b1 must have a random access iterator.");
    static_assert(std::is_same<typename std::iterator_traits<typename U_cont::iterator>::iterator_category,
                  std::random_access_iterator_tag>::value,
                  "Argument b2 must have a random access iterator.");
    return std::equal(b1.begin(), b1.end(), b2.begin());
}

#endif
