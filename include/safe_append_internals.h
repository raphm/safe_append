
// You do not need to include this file unless you are running the tests.

#ifndef safe_append_cpp_safe_append_internals_h
#define safe_append_cpp_safe_append_internals_h

#include <array>
#include <vector>
#include <type_traits>
#include <fstream>

#include "sha512.h"
#include "sha1.h"
#include "byte_utils.h"

std::string make_path(std::string const & pathname, std::string const & filename);
std::string get_name(std::string const & filepath);
std::string get_path(std::string const & filepath);
bool delete_file(std::string const & filepath);
bool mk_dir(std::string const & dirname);
bool rm_dir(std::string const & dirname);
long flen(std::string const & filepath);

template<typename T>
bool splatfile(std::string const & filepath, std::vector<T> const & new_contents, bool append=false) {
    std::ofstream::openmode flags;
    flags = std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
    if(append) {
        flags = std::ios_base::binary | std::ios_base::out | std::ios_base::app;
    }
    std::ofstream f(filepath, flags);
    if(f.is_open()) {
        f.write(reinterpret_cast<const char *>(new_contents.data()), (sizeof(T)*new_contents.size()));
        f.flush();
        f.close();
        return true;
    }
    return false;
}

template<typename T>
bool splatfile(std::string const & filepath, std::string const & new_contents, bool append=false) {
    std::ofstream::openmode flags;
    flags = std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
    if(append) {
        flags = std::ios_base::binary | std::ios_base::out | std::ios_base::app;
    }
    std::ofstream f(filepath, flags);
    if(f.is_open()) {
        f.write(reinterpret_cast<const char *>(new_contents.data()), new_contents.length());
        f.flush();
        f.close();
        return true;
    }
    return false;
}

typedef std::array<byte, SHA1::DIGEST_SIZE> uchar_sha_array;

template <typename T>
void internal_sha1(const unsigned char * msg_array, unsigned int length, uchar_sha_array::iterator & output_it)
{
    SHA1 sha1;
    std::array<unsigned int, 5> digest_uint;
    sha1.Input( msg_array, length );
    sha1.Result( digest_uint.data() );
    for(unsigned int ui : digest_uint) {
        encode_big_endian<uchar_sha_array::iterator>(output_it, ui);
        output_it+=(sizeof(uint32_t));
    }
}

template <typename T>
uchar_sha_array sha1(std::vector<T> const & input)
{
    uchar_sha_array output;
    uchar_sha_array::iterator it = output.begin();
    internal_sha1<T>(input.data(), (input.size()*sizeof(T)), it);
    return output;
}

template <typename T>
uchar_sha_array sha1(std::string const & input)
{
    uchar_sha_array output;
    uchar_sha_array::iterator it = output.begin();
    internal_sha1<T>(reinterpret_cast<const unsigned char *>(input.data()), input.size(), it);
    return output;
}

typedef std::array<byte, SHA512::DIGEST_SIZE>          uchar_sha512_array;

template<typename T>
uchar_sha512_array sha512(std::vector<T> const & input)
{
    uchar_sha512_array digest;
    SHA512 ctx = SHA512();
    ctx.init();
    ctx.update(reinterpret_cast<const unsigned char *>(input.data()), (input.size()*sizeof(T)));
    ctx.final(digest.data());
    return digest;
}

template<typename T>
uchar_sha512_array sha512(std::string const & input)
{
    uchar_sha512_array digest;
    SHA512 ctx = SHA512();
    ctx.init();
    ctx.update(reinterpret_cast<const unsigned char *>(input.data()), input.size());
    ctx.final(digest.data());
    return digest;
}


bool write_checksummed_file(std::string const & filename, std::vector<byte> const & bytes);
std::tuple<bool, std::vector<byte>, std::vector<byte>> read_checksummed_file(std::string const & filename);
std::string journal_name(std::string const & filepath);
bool create_append_journal(std::string const & filepath);
sa::status_value read_append_journal(std::string const & filepath, long & out_length);

#endif
