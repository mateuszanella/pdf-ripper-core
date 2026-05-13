#pragma once

#include <exception>
#include <string>
#include <utility>

namespace ripper::pdf::core
{
    /// Base exception type for all PDF document related errors.
    class exception : public std::exception
    {
    public:
        explicit exception(std::string message) : message_(std::move(message)) {}

        [[nodiscard]] const char *what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    /// Generic I/O exception for read/write/open failures.
    class io_exception : public exception
    {
    public:
        explicit io_exception(std::string message)
            : exception(std::move(message))
        {
        }
    };

    /// Generic parse exception for malformed PDF data.
    class parse_exception : public exception
    {
    public:
        explicit parse_exception(std::string message)
            : exception(std::move(message))
        {
        }
    };

    /// Generic logic exception for invalid API state or invariants.
    class logic_exception : public exception
    {
    public:
        explicit logic_exception(std::string message)
            : exception(std::move(message))
        {
        }
    };
}
