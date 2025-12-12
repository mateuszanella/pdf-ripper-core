#include "Core/PDF.hpp"

#include <stdexcept>
#include <utility>
#include <filesystem>

namespace Ripper::Core
{
    namespace
    {
        std::string CanonicalizePath(std::filesystem::path path)
        {
            auto canonicalPath = std::filesystem::weakly_canonical(path);

            return canonicalPath.generic_string();
        }
    }

    PDF::PDF(std::filesystem::path path)
        : m_path{CanonicalizePath(std::move(path))},
          m_file{m_path, std::ios::binary}
    {
        if (!m_file.is_open())
        {
            throw std::runtime_error{"Failed to open PDF file: " + m_path};
        }
    }

    std::string_view PDF::GetPath() const noexcept
    {
        return m_path;
    }

    bool PDF::IsOpen() const noexcept
    {
        return m_file.is_open();
    }
}
