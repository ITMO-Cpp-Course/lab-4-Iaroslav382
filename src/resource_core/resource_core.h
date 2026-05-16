
#include <memory>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

class ResourceError : public std::exception
{
  private:
    std::string message_;

  public:
    explicit ResourceError(const std::string& msg);
    const char* what() const noexcept override;
};

class FileHandle
{
  private:
    FILE* file_;
    std::string filename_;

  public:
    FileHandle() noexcept;
    explicit FileHandle(const std::string& filename, const std::string& mode);
    ~FileHandle() noexcept;

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;

    void open(const std::string& filename, const std::string& mode);
    void close() noexcept;
    bool is_open() const noexcept;
    const std::string& filename() const noexcept;
    FILE* get() const noexcept;

    void write(const std::string& data);
    std::string read_all();
    std::string read_line();
    void rewind();
};

class ResourceManager
{
  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;

  public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::shared_ptr<FileHandle> get(const std::string& filename, const std::string& mode);
    bool remove(const std::string& filename);
    size_t cleanup();
    bool has(const std::string& filename) const;
    size_t size() const;
};

} // namespace lab4::resource
