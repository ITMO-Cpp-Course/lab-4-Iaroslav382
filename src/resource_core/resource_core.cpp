#include "resource_core.h"
#include <cerrno>
#include <cstring>

namespace lab4::resource
{

ResourceError::ResourceError(const std::string& msg) : message_(msg) {}

const char* ResourceError::what() const noexcept
{
    return message_.c_str();
}

FileHandle::FileHandle() noexcept : file_(nullptr), filename_() {}

FileHandle::FileHandle(const std::string& filename, const std::string& mode) : file_(nullptr), filename_()
{
    open(filename, mode);
}

FileHandle::~FileHandle() noexcept
{
    close();
}

FileHandle::FileHandle(FileHandle&& other) noexcept : file_(other.file_), filename_(std::move(other.filename_))
{
    other.file_ = nullptr;
    other.filename_.clear();
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        file_ = other.file_;
        filename_ = std::move(other.filename_);
        other.file_ = nullptr;
        other.filename_.clear();
    }
    return *this;
}

void FileHandle::open(const std::string& filename, const std::string& mode)
{
    close();

    file_ = std::fopen(filename.c_str(), mode.c_str());
    if (!file_)
    {
        throw ResourceError("Cannot open file: " + filename + " (" + std::strerror(errno) + ")");
    }
    filename_ = filename;
}

void FileHandle::close() noexcept
{
    if (file_)
    {
        std::fclose(file_);
        file_ = nullptr;
        filename_.clear();
    }
}

bool FileHandle::is_open() const noexcept
{
    return file_ != nullptr;
}

const std::string& FileHandle::filename() const noexcept
{
    return filename_;
}

FILE* FileHandle::get() const noexcept
{
    return file_;
}

void FileHandle::write(const std::string& data)
{
    if (!file_)
    {
        throw ResourceError("File not open: " + filename_);
    }

    size_t written = std::fwrite(data.c_str(), 1, data.size(), file_);
    if (written != data.size())
    {
        throw ResourceError("Write failed: " + filename_);
    }
    std::fflush(file_);
}

std::string FileHandle::read_all()
{
    if (!file_)
    {
        throw ResourceError("File not open: " + filename_);
    }

    long pos = std::ftell(file_);
    if (pos == -1)
    {
        throw ResourceError("Failed to get file position: " + filename_);
    }

    if (std::fseek(file_, 0, SEEK_END) != 0)
    {
        throw ResourceError("Failed to seek: " + filename_);
    }

    long size = std::ftell(file_);
    if (size == -1)
    {
        throw ResourceError("Failed to get file size: " + filename_);
    }

    if (std::fseek(file_, pos, SEEK_SET) != 0)
    {
        throw ResourceError("Failed to restore position: " + filename_);
    }

    if (size == 0)
    {
        return "";
    }

    std::string result;
    result.resize(static_cast<size_t>(size));
    size_t read = std::fread(&result[0], 1, static_cast<size_t>(size), file_);

    if (read != static_cast<size_t>(size))
    {
        throw ResourceError("Read failed: " + filename_);
    }

    return result;
}

std::string FileHandle::read_line()
{
    if (!file_)
    {
        throw ResourceError("File not open: " + filename_);
    }

    char buffer[4096];
    if (std::fgets(buffer, sizeof(buffer), file_))
    {
        std::string line = buffer;
        // Удаляем символ новой строки
        if (!line.empty() && line.back() == '\n')
        {
            line.pop_back();
        }
        return line;
    }

    return "";
}

void FileHandle::rewind()
{
    if (file_)
    {
        std::rewind(file_);
    }
}

std::shared_ptr<FileHandle> ResourceManager::get(const std::string& filename, const std::string& mode)
{
    auto it = cache_.find(filename);
    if (it != cache_.end())
    {
        auto ptr = it->second.lock();
        if (ptr)
        {
            return ptr;
        }
    }

    auto ptr = std::make_shared<FileHandle>(filename, mode);
    cache_[filename] = ptr;
    return ptr;
}

bool ResourceManager::remove(const std::string& filename)
{
    return cache_.erase(filename) > 0;
}

size_t ResourceManager::cleanup()
{
    size_t removed = 0;
    auto it = cache_.begin();

    while (it != cache_.end())
    {
        if (it->second.expired())
        {
            it = cache_.erase(it);
            removed++;
        }
        else
        {
            ++it;
        }
    }

    return removed;
}

bool ResourceManager::has(const std::string& filename) const
{
    auto it = cache_.find(filename);
    if (it == cache_.end())
    {
        return false;
    }
    return !it->second.expired();
}

size_t ResourceManager::size() const
{
    size_t count = 0;
    for (const auto& pair : cache_)
    {
        if (!pair.second.expired())
        {
            count++;
        }
    }
    return count;
}

} // namespace lab4::resource