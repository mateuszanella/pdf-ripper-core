#include "core/document.hpp"

#include <fstream>
#include <memory>

#include "core/reader/file_reader.hpp"
#include "core/parser/parser.hpp"

namespace ripper::core
{
    document::document(std::unique_ptr<class reader> reader)
        : reader_(std::move(reader)),
          parser_(std::make_unique<class parser>(*this, *reader_))
    {
    }

    document::document(const std::filesystem::path &path)
        : document(std::make_unique<ripper::core::file_reader>(path))
    {
    }

    document document::open(const std::filesystem::path &path)
    {
        return document{path};
    }

    reader &document::reader() const noexcept
    {
        return *reader_;
    }

    parser &document::parser() const noexcept
    {
        return *parser_;
    }
}
