#pragma once

#include <filesystem>
#include <string>
#include <utility>

namespace test_fixture
{
namespace fs = std::filesystem;

inline fs::path project_root()
{
    return fs::path{__FILE__}.parent_path().parent_path();
}

inline fs::path fixture_pdf_path()
{
    return project_root() / "tests" / "fixtures" / "pdfs" / "test.pdf";
}

inline fs::path temp_pdf_path(std::string filename)
{
    return fs::temp_directory_path() / std::move(filename);
}

class scoped_temp_file
{
public:
    explicit scoped_temp_file(std::string filename)
        : path_(fs::temp_directory_path() / std::move(filename))
    {
        fs::remove(path_);
    }

    ~scoped_temp_file()
    {
        fs::remove(path_);
    }

    scoped_temp_file(const scoped_temp_file&) = delete;
    scoped_temp_file& operator=(const scoped_temp_file&) = delete;

    scoped_temp_file(scoped_temp_file&&) noexcept = default;
    scoped_temp_file& operator=(scoped_temp_file&&) noexcept = default;

    [[nodiscard]] const fs::path& path() const
    {
        return path_;
    }

private:
    fs::path path_;
};
} // namespace test_fixture
