//
// Created by myselfleo on 23/11/2026.
//

#pragma once
#include <stdexcept>
#include <string>
#include <utility>

#define wrld_try(expr) if (auto _msfl_try_result = (expr); _msfl_try_result.is_err()) {return _msfl_try_result.as_err();}

#define wrld_try_assign(varname, expr) auto var##_res = (expr); if (var##_res.is_err()) return var##_res.as_err(); auto var = std::move(*var##_res);

namespace wrld {

    enum class ErrorType {
        Unspecified,
        GlError,
        FileError,
        ShaderError,
    };

    class Error {
    public:
        explicit Error(ErrorType type, std::string msg = "");

        [[nodiscard]] ErrorType get_type() const;
        [[nodiscard]] std::string get_msg() const;

    private:
        ErrorType type;
        std::string msg;
    };


    template<typename T>
    class Result {
    public:
        Result(const T& value): _value(value), _is_err(false) {}
        Result(Error  err): _err(std::move(err)), _is_err(true) {}
        Result(const ErrorType type, const std::string& msg = ""): _err(Error(type, msg)), _is_err(true) {}

        [[nodiscard]] bool is_ok() const {
            return !_is_err;
        }
        [[nodiscard]] bool is_err() const {
            return _is_err;
        }

        [[nodiscard]] Error as_err() const {
            if (!is_err()) throw std::runtime_error("Tried to access error of a non-erroneous Result object");
            return _err;
        }

        T as_ok() const {
            if (is_err()) throw std::runtime_error("Tried to access value of an erroneous Result object");
            return _value;
        }

    private:
        union {
            T _value;
            Error _err;
        };
        bool _is_err;
    };

    template<>
    class Result<void> {
    public:
        Result(): _err(Error(ErrorType::Unspecified, "")), _is_err(false) {}
        Result(Error  err): _err(std::move(err)), _is_err(true) {}
        Result(const ErrorType type, const std::string& msg = ""): _err(Error(type, msg)), _is_err(true) {}

        [[nodiscard]] bool is_ok() const {
            return !_is_err;
        }
        [[nodiscard]] bool is_err() const {
            return _is_err;
        }

        [[nodiscard]] Error as_err() const {
            if (!is_err()) throw std::runtime_error("Tried to access error of a non-erroneous Result object");
            return _err;
        }

    private:
        Error _err;
        bool _is_err;
    };

}
