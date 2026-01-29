#pragma once

#include <filesystem>
#include <memory>

#include "Core/Parser/Parser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class PDF
    {
    public:
        explicit PDF(std::unique_ptr<Ripper::Core::Reader> reader);
        explicit PDF(const std::filesystem::path path);

        static PDF Open(const std::filesystem::path path);

        [[nodiscard]] Ripper::Core::Reader &Reader() noexcept;
        [[nodiscard]] const Ripper::Core::Reader &Reader() const noexcept;

        [[nodiscard]] Ripper::Core::Parser &Parser() noexcept;
        [[nodiscard]] const Ripper::Core::Parser &Parser() const noexcept;

    private:
        std::unique_ptr<Ripper::Core::Reader> _reader;
        std::unique_ptr<Ripper::Core::Parser> _parser;
    };
}
