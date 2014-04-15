
#include <fstream>

#include <boost/filesystem.hpp>

#include "safe_append.h"
#include "safe_append_internals.h"

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


long flen(std::string const & filepath) {
    boost::system::error_code sec;
    long rv = boost::filesystem::file_size(boost::filesystem::path(filepath), sec);
    if(boost::system::errc::success == sec.value()) {
        return rv;
    } else {
        return -1;
    }
}

/**
 * Write (or overwrite, if file fname exists) the bytes to file fname,
 * preceded by the sha512 checksum of the bytes. Provides a reader a way
 * to ensure file is valid and that write succeeded fully.
 */

bool write_checksummed_file(std::string const & filename, std::vector<byte> const & bytes) {
    std::ofstream f(filename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    if(f.is_open()) {
        auto hashvec = sha512(bytes);
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
                std::get<0>(rv)=bytes_equal(sha512(std::get<2>(rv)), std::get<1>(rv));
            }
            f.close();
        }
    }
    return rv;
}

std::string journal_name(std::string const & filepath) {
    std::string const fname = get_name(filepath);
    std::string journal_name("j_");
    std::array<byte, SHA1::DIGEST_SIZE> hash = sha1<std::string>(fname);
    std::back_insert_iterator<std::string> it = std::back_inserter(journal_name);
    bytes_to_hex(hash.begin(), hash.end(), it);
    journal_name+=".jrn";
    return make_path(get_path(filepath), journal_name);
}

bool create_append_journal(std::string const & filepath) {
    if(!file_exists(filepath)) {
        return false;
    }
    std::vector<byte> bytes;
    int curlen = flen(filepath);
    if(curlen<0) return false;
    bytes.resize(sizeof(uint32_t));
    
    // The only data we are writing for now is the length of the file to be journaled.
    // This may (probably will) expand in the future.
    
    std::vector<byte>::iterator it = bytes.begin();
    encode_big_endian(it, (uint32_t)curlen);
    
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
    
    std::vector<byte>::iterator it = content.begin();
    out_length = extract_big_endian(it);
    
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



