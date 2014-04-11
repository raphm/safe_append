
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE safe_append_test
#include <boost/test/unit_test.hpp>

#include "safe_append.h"
#include "safe_append_internals.h"

#include <algorithm>

BOOST_AUTO_TEST_CASE( file_name_path_tests )
{
    {
        std::string test_file = make_path("test/", "foo.txt");
        BOOST_CHECK(0==test_file.compare("test/foo.txt"));
        BOOST_CHECK_EQUAL("foo.txt", get_name(test_file));
        BOOST_CHECK_EQUAL("test", get_path(test_file));
    }
    
    {
        std::string test_file = make_path("/tmp/bar/baz", "foo.txt");
        BOOST_CHECK(0==test_file.compare("/tmp/bar/baz/foo.txt"));
        BOOST_CHECK_EQUAL("foo.txt", get_name(test_file));
        BOOST_CHECK_EQUAL("/tmp/bar/baz", get_path(test_file));
    }
    
    {
        std::string test_file = make_path("", "foo.txt");
        BOOST_CHECK(0==test_file.compare("foo.txt"));
        BOOST_CHECK_EQUAL("foo.txt", get_name(test_file));
        BOOST_CHECK_EQUAL("", get_path(test_file));
    }
    
}

BOOST_AUTO_TEST_CASE( sha1_test_cases )
{
    const int digest_size = SHA1::DIGEST_SIZE;
    
    {
        std::string input("abc");
        std::string uc_hash("A9993E364706816ABA3E25717850C26C9CD0D89D");
        std::string lc_hash(uc_hash);
        std::transform(lc_hash.begin(), lc_hash.end(), lc_hash.begin(), ::tolower);
        
        std::vector<byte> uctest = hex_to_bytes(uc_hash);
        std::vector<byte> lctest = hex_to_bytes(lc_hash);
        
        BOOST_CHECK(bytes_equal(uctest, lctest));
        
        std::vector<byte> hashvec = as_vec(sha1(as_vec(input)));
        
        BOOST_CHECK(bytes_equal(lctest, hashvec));
        
        BOOST_CHECK_EQUAL(hashvec.size(), digest_size);
    }
    
    {
        
        std::string input("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
        std::string uc_hash("84983E441C3BD26EBAAE4AA1F95129E5E54670F1");
        std::string lc_hash(uc_hash);
        std::transform(lc_hash.begin(), lc_hash.end(), lc_hash.begin(), ::tolower);
        
        std::vector<byte> uctest = hex_to_bytes(uc_hash);
        std::vector<byte> lctest = hex_to_bytes(lc_hash);
        
        BOOST_CHECK(bytes_equal(uctest, lctest));
        
        std::vector<byte> hashvec = as_vec(sha1(as_vec(input)));
        
        BOOST_CHECK(bytes_equal(lctest, hashvec));
        
        BOOST_CHECK_EQUAL(hashvec.size(), digest_size);
        
    }
    
}

BOOST_AUTO_TEST_CASE( sha512_test_cases ) {
    
    const int digest_size = SHA512::DIGEST_SIZE; // not visible inside boost test case macros...
    
    // Tests from http://csrc.nist.gov/groups/ST/toolkit/examples.html#aHashing
    
    {
        std::string input("abc");
        
        std::string uc_hash("DDAF35A193617ABACC417349AE204131"
                            "12E6FA4E89A97EA20A9EEEE64B55D39A"
                            "2192992A274FC1A836BA3C23A3FEEBBD"
                            "454D4423643CE80E2A9AC94FA54CA49F");
        
        std::string lc_hash(uc_hash);
        std::transform(lc_hash.begin(), lc_hash.end(), lc_hash.begin(), ::tolower);
        
        std::vector<byte> uctest = hex_to_bytes(uc_hash);
        std::vector<byte> lctest = hex_to_bytes(lc_hash);
        
        BOOST_CHECK(bytes_equal(uctest, lctest));
        
        std::vector<byte> hashvec = as_vec(sha512(as_vec(input)));
        
        BOOST_CHECK(bytes_equal(lctest, hashvec));
        
        BOOST_CHECK_EQUAL(hashvec.size(), digest_size);
        
    }
    
    {
        
        std::string input = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
        
        std::string uc_hash("8E959B75DAE313DA8CF4F72814FC143F"
                            "8F7779C6EB9F7FA17299AEADB6889018"
                            "501D289E4900F7E4331B99DEC4B5433A"
                            "C7D329EEB6DD26545E96E55B874BE909");
        
        std::string lc_hash(uc_hash);
        std::transform(lc_hash.begin(), lc_hash.end(), lc_hash.begin(), ::tolower);
        
        std::vector<byte> uctest = hex_to_bytes(uc_hash);
        std::vector<byte> lctest = hex_to_bytes(lc_hash);
        
        BOOST_CHECK(bytes_equal(uctest, lctest));
        
        std::vector<byte> hashvec = as_vec(sha512(as_vec(input)));
        
        BOOST_CHECK(bytes_equal(lctest, hashvec));
        
        BOOST_CHECK_EQUAL(hashvec.size(), digest_size);
        
    }
    
}

BOOST_AUTO_TEST_CASE( checksummed_file_tests )
{
    mk_dir("test/");
    
    std::vector<byte> test_bytes = as_vec(std::string("This is a test of checksumming."));
    std::vector<byte> master_cksum = as_vec(sha512(test_bytes));
    
    std::string test_file = make_path("test", "foo.txt");
    
    BOOST_CHECK(0==test_file.compare("test/foo.txt"));
    BOOST_CHECK(write_checksummed_file(test_file, test_bytes));
    BOOST_CHECK_EQUAL(flen(test_file) , test_bytes.size()+SHA512::DIGEST_SIZE);
    
    std::tuple<bool, std::vector<byte>, std::vector<byte>> tpl = read_checksummed_file(test_file);
    
    BOOST_CHECK_EQUAL(std::get<0>(tpl), true);
    BOOST_CHECK(bytes_equal(std::get<1>(tpl), master_cksum));
    BOOST_CHECK(bytes_equal(std::get<2>(tpl), test_bytes));
    
    delete_file("test/foo.txt");
    rm_dir("test");
    
}

BOOST_AUTO_TEST_CASE( journal_name_tests )
{
    std::string input("abc");
    std::string uc_hash("A9993E364706816ABA3E25717850C26C9CD0D89D");
    std::string lc_hash(uc_hash);
    std::transform(lc_hash.begin(), lc_hash.end(), lc_hash.begin(), ::tolower);
    
    std::vector<byte> uctest = hex_to_bytes(uc_hash);
    std::vector<byte> lctest = hex_to_bytes(lc_hash);
    
    BOOST_CHECK(bytes_equal(uctest, lctest));
    
    std::string jrn_name = journal_name(make_path("/tmp/foo", input));
    
    BOOST_CHECK_EQUAL("/tmp/foo/j_"+bytes_to_hex(lctest)+".jrn", jrn_name);
    
    BOOST_CHECK_EQUAL(journal_name("/tmp/foo/baz.txt"), "/tmp/foo/j_3c6abbba2b09ba4928dcf77610a8124d79b184f7.jrn");
}

BOOST_AUTO_TEST_CASE( journal_creation_tests )
{
    mk_dir("test/");
    
    std::string fname("test/tmp.txt");
    
    splatfile(fname, "append test\n");
    long orig_len = flen(fname);
    create_append_journal(fname);
    long journaled_len = 0;
    BOOST_CHECK_EQUAL(sa::hot, read_append_journal(fname, journaled_len));
    BOOST_CHECK_EQUAL(journaled_len, orig_len);
    BOOST_CHECK_EQUAL(sa::hot, sa::status(fname));
    
    std::string additional_contents("line 2\n");
    splatfile(fname, additional_contents, true);
    BOOST_CHECK_EQUAL(sa::hot, read_append_journal(fname, journaled_len));
    BOOST_CHECK_EQUAL(journaled_len, orig_len);
    BOOST_CHECK_NE(orig_len, flen(fname));
    BOOST_CHECK_EQUAL(orig_len+additional_contents.length(), flen(fname));
    BOOST_CHECK_EQUAL(sa::hot, sa::status(fname));
    
    create_append_journal(fname);
    journaled_len = 0;
    BOOST_CHECK_EQUAL(sa::hot, read_append_journal(fname, journaled_len));
    BOOST_CHECK_NE(orig_len, flen(fname));
    BOOST_CHECK_EQUAL(orig_len+additional_contents.length(), flen(fname));
    BOOST_CHECK_EQUAL(orig_len+additional_contents.length(), journaled_len);
    BOOST_CHECK_EQUAL(sa::hot, sa::status(fname));
    
    splatfile(journal_name(fname), "x");
    journaled_len = 0;
    BOOST_CHECK_EQUAL(sa::dirty, read_append_journal(fname, journaled_len));
    BOOST_CHECK_EQUAL(-1, journaled_len);
    BOOST_CHECK_EQUAL(sa::dirty, sa::status(fname));
    
    journaled_len = 0;
    delete_file(journal_name(fname));
    BOOST_CHECK_EQUAL(sa::clean, read_append_journal(fname, journaled_len));
    BOOST_CHECK_EQUAL(sa::clean, sa::status(fname));
    
    delete_file(fname);
    
    BOOST_CHECK_EQUAL(sa::clean, read_append_journal(fname, journaled_len));
    BOOST_CHECK_EQUAL(sa::clean, sa::status(fname));
    
    rm_dir("test/");
}

BOOST_AUTO_TEST_CASE( sa_function_tests )
{
    mk_dir("test/");
    
    std::string fname("test/tmp.txt");
    
    splatfile(fname, "append test\n");
    
    long orig_len = flen(fname);
    
    BOOST_CHECK(sa::start(fname));
    
    BOOST_CHECK_EQUAL(sa::hot, sa::status(fname));
    
    BOOST_CHECK(!sa::cleanup(fname));
    
    BOOST_CHECK(!sa::start(fname));
    
    splatfile(fname, "x", true);
    
    BOOST_CHECK_EQUAL(sa::hot, sa::status(fname));
    
    BOOST_CHECK_NE(flen(fname), orig_len);
    
    BOOST_CHECK(sa::rollback(fname));
    
    BOOST_CHECK_EQUAL(sa::clean, sa::status(fname));
    
    BOOST_CHECK_EQUAL(flen(fname), orig_len);
    
    splatfile(journal_name(fname), "x", true);
    
    BOOST_CHECK_EQUAL(sa::dirty, sa::status(fname));
    
    BOOST_CHECK(!sa::start(fname));
    
    BOOST_CHECK(!sa::rollback(fname));
    
    BOOST_CHECK(sa::cleanup(fname));
    
    BOOST_CHECK_EQUAL(sa::clean, sa::status(fname));
    
    rm_dir("test/");
    
}


