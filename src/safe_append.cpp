
#include <fstream>

#include <boost/filesystem.hpp>

#include "safe_append.h"
#include "safe_append_internals.h"

inline bool system_is_big_endian()
{
    short word = 0x8421;
    return ( (*(unsigned char *)(&word)) == 0x84 );
}

inline uint32_t swap_endian (uint32_t n)
{
    return (((n&0x000000FF)<<24)+((n&0x0000FF00)<<8)+((n&0x00FF0000)>>8)+((n&0xFF000000)>>24));
}

inline uint32_t uint_native_to_be(uint32_t n) {
    if(system_is_big_endian()) {
        return n;
    } else {
        return swap_endian(n);
    }
}

inline uint32_t uint_be_to_native(uint32_t n) {
    if(system_is_big_endian()) {
        return n;
    } else {
        return swap_endian(n);
    }
}

inline char nibble_to_hex(byte b) {
    b&=0x0F;
    if(b<10) {
        return '0'+b;
    } else {
        return 'a'+(b-10);
    }
}

inline char hinibble(byte b) { return nibble_to_hex(((b&0xF0)>>4)); }
inline char lonibble(byte b) { return nibble_to_hex(b); }

std::string bytes_to_hex(std::vector<byte> const & bytes) {
    std::string hexstr;
    
    hexstr.resize(bytes.size()*2);
    
    for(int i=0; i<bytes.size(); i+=1) {
        hexstr[(i*2)]=hinibble(bytes[i]);
        hexstr[(i*2)+1]=lonibble(bytes[i]);
    }
    
    return hexstr;
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

inline byte hex_to_byte(char hinibble, char lonibble) {
    byte b = hex_to_nibble(lonibble);
    b |= (0xF0 & (hex_to_nibble(hinibble))<<4);
    return b;
}

std::vector<byte> hex_to_bytes(std::string const & str) {
    std::vector<byte> data;
    data.resize(str.length()/2);
    for(int i=0;i<str.length();i+=2) {
        data[i/2]=(byte)hex_to_byte(str[i], str[i+1]);
    }
    return data;
}

bool bytes_equal(std::vector<byte> const & b1, std::vector<byte> const & b2) {
    return std::equal(b1.begin(), b1.end(), b2.begin());
}

bool mk_dir(std::string const & dirname) {
    boost::system::error_code sec;
    bool success = boost::filesystem::create_directory(boost::filesystem::path(dirname), sec);
    return success && boost::system::errc::success == sec.value();
}

bool rm_dir(std::string const & dirname) {
    boost::system::error_code sec;
    bool success = boost::filesystem::remove_all(boost::filesystem::path(dirname), sec);
    return success && boost::system::errc::success == sec.value();
}

// filepath is /foo/bar/baz.txt
// pathname of /foo/bar/baz.txt is /foo/bar
// filename of /foo/bar/baz.txt is baz.txt

std::string make_path(std::string const & pathname, std::string const & filename) {
    boost::filesystem::path p(pathname);
    p /= filename;
    return p.make_preferred().string();
}

std::string get_name(std::string const & filepath) {
    boost::filesystem::path p(filepath);
    return p.filename().string();
}

std::string get_path(std::string const & filepath) {
    boost::filesystem::path p(filepath);
    return p.parent_path().string();
}

bool file_exists(std::string const & filepath) {
    boost::filesystem::path p(filepath);
    boost::system::error_code sec;
    return exists(p, sec) && boost::system::errc::success == sec.value();
}

bool delete_file(std::string const & filepath) {
    boost::system::error_code sec;
    bool success = boost::filesystem::remove(boost::filesystem::path(filepath), sec);
    return success && boost::system::errc::success == sec.value();
}

bool splatfile(std::string const & filepath, std::string const & new_contents, bool append) {
    std::ofstream::openmode flags;
    flags = std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;
    if(append) {
        flags = std::ios_base::binary | std::ios_base::out | std::ios_base::app;
    }
    std::ofstream f(filepath, flags);
    if(f.is_open()) {
        std::vector<byte> bytes = as_vec(new_contents);
        f.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        f.flush();
        f.close();
        return true;
    }
    return false;
}

long flen(std::string const & filepath) {
    boost::system::error_code sec;
    long rv = boost::filesystem::file_size(boost::filesystem::path(filepath), sec);
    if(boost::system::errc::success == sec.value()) {
        return rv;
    } else {
        return -1;
    }
}

std::array<byte, SHA1::DIGEST_SIZE> sha1(std::vector<byte> const & input)
{
    
    typedef std::array<unsigned int, 5>                  uint_sha_array;
    typedef std::array<byte, SHA1::DIGEST_SIZE>          uchar_sha_array;
    
    SHA1 sha1;
    uchar_sha_array  digest_byte;
    
    sha1.Input( input.data(), input.size() );
    
    sha1.Result(reinterpret_cast<unsigned int *>(digest_byte.data()));
    
    if(!system_is_big_endian()) {
        for(uint_sha_array::iterator it = reinterpret_cast<uint_sha_array::iterator>(digest_byte.begin()) ;
            it < reinterpret_cast<uint_sha_array::iterator>(digest_byte.end()) ;
            it+=1 ) {
            *it = swap_endian(*it);
        }
    }
    
    return digest_byte;
}

std::array<byte, SHA512::DIGEST_SIZE>  sha512(std::vector<byte> const & input)
{
    
    std::array<byte, SHA512::DIGEST_SIZE>  digest;
    
    std::fill(digest.begin(), digest.end(), 0);
    
    SHA512 ctx = SHA512();
    ctx.init();
    ctx.update((unsigned char*)input.data(), input.size());
    ctx.final(digest.data());
    
    return digest;
}

/**
 * Write (or overwrite, if file fname exists) the bytes to file fname,
 * preceded by the sha512 checksum of the bytes. Provides a reader a way
 * to ensure file is valid and that write succeeded fully.
 */

bool write_checksummed_file(std::string const & filename, std::vector<byte> const & bytes) {
    std::ofstream f(filename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    if(f.is_open()) {
        std::vector<byte> hashvec = as_vec(sha512(bytes));
        f.write(reinterpret_cast<const char *>(hashvec.data()), hashvec.size());
        f.flush();
        f.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        f.flush();
        f.close();
        if (f.rdstate() == f.goodbit) {
            return true;
        }
    }
    return false;
}

std::tuple<bool, std::vector<byte>, std::vector<byte>> read_checksummed_file(std::string const & filename) {
    long len = flen(filename);
    std::tuple<bool, std::vector<byte>, std::vector<byte>> rv;
    
    std::get<0>(rv)=false;    // cksum of contents valid?
    std::get<1>(rv).clear();  // cksum
    std::get<2>(rv).clear();  // contents
    
    if(len>SHA512::DIGEST_SIZE) {
        std::ifstream f(filename, f.binary | f.in);
        if(f.is_open()) {
            f.seekg (0, f.beg);
            std::get<1>(rv).resize(SHA512::DIGEST_SIZE);
            f.read(reinterpret_cast<char *>(std::get<1>(rv).data()), SHA512::DIGEST_SIZE);
            
            len-=SHA512::DIGEST_SIZE;
            std::get<2>(rv).resize(len);
            f.read(reinterpret_cast<char *>(std::get<2>(rv).data()), len);
            
            if(f.fail()) {
                std::get<0>(rv)=false;
            } else {
                std::get<0>(rv)=bytes_equal(as_vec(sha512(std::get<2>(rv))), std::get<1>(rv));
            }
            f.close();
        }
    }
    return rv;
}

std::string journal_name(std::string const & filepath) {
    std::string const fname = get_name(filepath);
    std::string const fpath = get_path(filepath);
    std::string const journal_name("j_" + bytes_to_hex(as_vec(sha1(as_vec(fname))))+".jrn");
    return make_path(fpath, journal_name);
}

bool create_append_journal(std::string const & filepath) {
    if(!file_exists(filepath)) {
        return false;
    }
    int curlen = uint_native_to_be(flen(filepath));
    std::vector<byte> bytes;
    bytes.resize(sizeof(curlen));
    std::copy(reinterpret_cast<byte *>(&curlen), reinterpret_cast<byte *>(&curlen+sizeof(curlen)), bytes.begin());
    return write_checksummed_file(journal_name(filepath), bytes);
}

sa::status_value read_append_journal(std::string const & filepath, long & out_length) {
    std::string jname = journal_name(filepath);
    
    if(!file_exists(jname)) {
        return sa::clean;
    }
    
    std::tuple<bool, std::vector<byte>, std::vector<byte>> tpl = read_checksummed_file(jname);
    
    bool valid = std::get<0>(tpl);
    std::vector<byte> cksum = std::get<1>(tpl);
    std::vector<byte> content = std::get<2>(tpl);
    
    if(!valid) {
        out_length = -1;
        return sa::dirty;
    }
    
    uint32_t jrn_len = 0;
    std::copy(content.begin(), content.end(), reinterpret_cast<byte *>(&jrn_len));
    
    out_length = uint_be_to_native(jrn_len);
    
    return sa::hot;
}

bool delete_append_journal(std::string const & filepath) {
    std::string jrn_name = journal_name(filepath);
    if(!file_exists(jrn_name)) return true;
    return delete_file(jrn_name);
}

sa::status_value sa::status(std::string const & filepath) {
    long unused;
    return read_append_journal(filepath, unused);
}

bool sa::start(std::string const & filepath) {
    if(sa::status(filepath)!=sa::clean) {
        return false;
    }
    return create_append_journal(filepath);
}

bool sa::commit(std::string const & filepath) {
    if(sa::status(filepath)!=sa::hot) {
        return false;
    }
    return delete_append_journal(filepath);
}

bool sa::cleanup(std::string const & filepath) {
    if(sa::status(filepath)!=sa::dirty) {
        return false;
    }
    return delete_append_journal(filepath);
}

bool sa::rollback(std::string const & filepath) {
    long valid_length = 0;
    sa::status_value s = read_append_journal(filepath, valid_length);
    if(s!=sa::hot || valid_length<0) {
        return false;
    }
    long current_length = flen(filepath);
    
    if(valid_length>=current_length) {
        // Do not touch file. We do not want to expand the already bad data!
        // Valid length should have been less than the current length.
        return false;
    }
    
    boost::system::error_code sec;
    boost::filesystem::resize_file(filepath, valid_length, sec);
    if(boost::system::errc::success == sec.value()) {
        return(delete_append_journal(filepath));
    } else {
        return (boost::system::errc::success == sec.value());
    }
}



