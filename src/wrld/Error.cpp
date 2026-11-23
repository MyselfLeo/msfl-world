//
// Created by myselfleo on 23/11/2026.
//

#include <wrld/Error.hpp>

namespace wrld {
    Error::Error(const ErrorType type, std::string msg): type(type), msg(std::move(msg)) {}

    ErrorType Error::get_type() const {
        return type;
    }

    std::string Error::get_msg() const {
        return msg;
    }
}
