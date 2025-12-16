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

        [[nodiscard]] Ripper::Core::Reader &GetReader() noexcept;
        [[nodiscard]] const Ripper::Core::Reader &GetReader() const noexcept;

        [[nodiscard]] Ripper::Core::Parser &GetParser() noexcept;
        [[nodiscard]] const Ripper::Core::Parser &GetParser() const noexcept;

    private:
        std::unique_ptr<Ripper::Core::Reader> _reader;
        std::unique_ptr<Ripper::Core::Parser> _parser;
    };
}
