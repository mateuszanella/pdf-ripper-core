#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace ripper::pdf::core
{

/// Represents a PDF numeric object — either an integer or a real.
///
/// Preserves both the parsed value and the original lexeme from the source
/// file, enabling lossless round-trip serialization (e.g. `1.50` stays
/// `1.50`, not `1.5`).
///
/// When constructed programmatically (without a lexeme), the serializer
/// falls back to standard numeric formatting.
class number_object
{
public:
    /// Discriminator indicating whether the value was parsed as an integer or real.
    enum class kind : std::uint8_t
    {
        integer,
        real
    };

    /// Construct a default integer zero.
    number_object() noexcept : kind_(kind::integer), value_(std::int64_t{0}) {}

    /// Construct an integer.  No original lexeme is stored; serialization
    /// will use standard integer formatting.
    explicit number_object(std::int64_t v) noexcept : kind_(kind::integer), value_(v) {}

    /// Construct a real.  No original lexeme is stored; serialization will
    /// use standard real formatting.
    explicit number_object(double v) : kind_(kind::real), value_(v) {}

    /// Construct an integer from parsed value + original source lexeme
    /// for round-trip fidelity.
    number_object(std::int64_t v, std::string lexeme) noexcept
        : kind_(kind::integer), value_(v), original_lexeme_(std::move(lexeme))
    {
    }

    /// Construct a real from parsed value + original source lexeme for
    /// round-trip fidelity.
    number_object(double v, std::string lexeme)
        : kind_(kind::real), value_(v), original_lexeme_(std::move(lexeme))
    {
    }

    /// Returns `true` if this number originated from an integer token.
    [[nodiscard]] bool is_integer() const noexcept
    {
        return kind_ == kind::integer;
    }

    /// Returns `true` if this number originated from a real token.
    [[nodiscard]] bool is_real() const noexcept
    {
        return kind_ == kind::real;
    }

    /// Returns the value as `std::int64_t`, truncating if the value was
    /// originally a real.
    [[nodiscard]] std::int64_t as_integer() const noexcept
    {
        return is_integer() ? std::get<std::int64_t>(value_) : static_cast<std::int64_t>(as_real());
    }

    /// Returns the value as `double`, widening if the value was originally
    /// an integer.
    [[nodiscard]] double as_real() const noexcept
    {
        return is_real() ? std::get<double>(value_)
                         : static_cast<double>(std::get<std::int64_t>(value_));
    }

    /// The original lexeme from the source file, if this number was
    /// constructed with provenance (parser path).  Empty for
    /// programmatically constructed numbers.
    [[nodiscard]] const std::string& original_lexeme() const noexcept
    {
        return original_lexeme_;
    }

private:
    kind kind_;
    std::variant<std::int64_t, double> value_;
    std::string original_lexeme_;
};

} // namespace ripper::pdf::core
