
// You do not need to include this file unless you are running the tests.

#ifndef safe_append_cpp_safe_append_internals_h
#define safe_append_cpp_safe_append_internals_h

#include <array>
#include <vector>
#include <type_traits>

#include "sha512.h"
#include "sha1.h"

typedef unsigned char byte;

template <typename T>
std::vector<byte> as_vec(T const & tcontainer) {
    static_assert(std::is_same<T, std::string>::value ||
                  std::is_same<T, std::array<byte, SHA1::DIGEST_SIZE>>::value ||
                  std::is_same<T, std::array<byte, SHA512::DIGEST_SIZE>>::value  ,
                  "as_vec() requires array or string");
    return std::vector<byte>(tcontainer.begin(), tcontainer.end());
}

std::string bytes_to_hex(std::vector<byte> const & bytes);
std::vector<byte> hex_to_bytes(std::string const & str);
bool bytes_equal(std::vector<byte> const & b1, std::vector<byte> const & b2);

std::string make_path(std::string const & pathname, std::string const & filename);
std::string get_name(std::string const & filepath);
std::string get_path(std::string const & filepath);
bool delete_file(std::string const & filepath);

bool mk_dir(std::string const & dirname);
bool rm_dir(std::string const & dirname);
bool splatfile(std::string const & filepath, std::string const & new_contents, bool append=false);
long flen(std::string const & filepath);

std::array<byte, SHA1::DIGEST_SIZE> sha1(std::vector<byte> const & input);
std::array<byte, SHA512::DIGEST_SIZE> sha512(std::vector<byte> const & input);

bool write_checksummed_file(std::string const & filename, std::vector<byte> const & bytes);
std::tuple<bool, std::vector<byte>, std::vector<byte>> read_checksummed_file(std::string const & filename);
std::string journal_name(std::string const & filepath);
bool create_append_journal(std::string const & filepath);
sa::status_value read_append_journal(std::string const & filepath, long & out_length);

#endif
