//
//  safe_append.h
//  safe-append-cpp
//
//  Created by Raphael Martelles on 4/11/14.
//
//

#ifndef __safe_append_cpp__safe_append__
#define __safe_append_cpp__safe_append__

#include <string>

namespace sa {
    
    enum status_value {
        clean, // no journal file exists
        dirty, // invalid journal file exists
        hot    // valid journal file exists but we don't know if write completed.
    };
    
    status_value status(std::string const & filepath);
    bool start(std::string const & filepath);
    bool commit(std::string const & filepath);
    bool cleanup(std::string const & filepath);
    bool rollback(std::string const & filepath);
}

#endif /* defined(__safe_append_cpp__safe_append__) */
