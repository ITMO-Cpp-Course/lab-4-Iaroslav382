#include "resource_core.h"
#include <catch2/catch_all.hpp>
#include <filesystem>

using namespace lab4::resource;
namespace fs = std::filesystem;

void make_file(const std::string& name, const std::string& content)
{
    FILE* f = fopen(name.c_str(), "w");
    if (f)
    {
        fwrite(content.c_str(), 1, content.size(), f);
        fclose(f);
    }
}

TEST_CASE("FileHandle - RAII автоматическое закрытие", "[FileHandle]")
{
    make_file("test1.txt", "hello");

    {
        FileHandle f("test1.txt", "r");
        REQUIRE(f.is_open());
        REQUIRE(f.filename() == "test1.txt");
    }

    REQUIRE(fs::remove("test1.txt"));
}

TEST_CASE("FileHandle - исключение при ошибке", "[FileHandle]")
{
    REQUIRE_THROWS_AS(FileHandle("nonexistent.txt", "r"), ResourceError);
}

TEST_CASE("FileHandle - move semantics", "[FileHandle]")
{
    make_file("test2.txt", "data");

    FileHandle f1("test2.txt", "r");
    REQUIRE(f1.is_open());

    FileHandle f2 = std::move(f1);
    REQUIRE_FALSE(f1.is_open());
    REQUIRE(f2.is_open());

    fs::remove("test2.txt");
}

TEST_CASE("FileHandle - запись и чтение", "[FileHandle]")
{
    {
        FileHandle f("test3.txt", "w");
        f.write("Hello\nWorld");
    }

    {
        FileHandle f("test3.txt", "r");
        REQUIRE(f.read_all() == "Hello\nWorld");
    }

    fs::remove("test3.txt");
}

TEST_CASE("FileHandle - построчное чтение", "[FileHandle]")
{
    make_file("test4.txt", "Line1\nLine2\nLine3");

    FileHandle f("test4.txt", "r");
    REQUIRE(f.read_line() == "Line1");
    REQUIRE(f.read_line() == "Line2");
    REQUIRE(f.read_line() == "Line3");
    REQUIRE(f.read_line() == "");

    fs::remove("test4.txt");
}

TEST_CASE("FileHandle - rewind", "[FileHandle]")
{
    make_file("test5.txt", "First\nSecond");

    FileHandle f("test5.txt", "r");
    REQUIRE(f.read_line() == "First");

    f.rewind();
    REQUIRE(f.read_line() == "First");

    fs::remove("test5.txt");
}

TEST_CASE("ResourceManager - кеширование", "[ResourceManager]")
{
    make_file("test6.txt", "cached");

    ResourceManager m;

    auto p1 = m.get("test6.txt", "r");
    REQUIRE(m.size() == 1);
    REQUIRE(m.has("test6.txt"));

    auto p2 = m.get("test6.txt", "r");
    REQUIRE(p1 == p2); // Один и тот же объект

    fs::remove("test6.txt");
}

TEST_CASE("ResourceManager - очистка", "[ResourceManager]")
{
    make_file("test7.txt", "data");

    ResourceManager m;

    auto p = m.get("test7.txt", "r");
    REQUIRE(m.size() == 1);

    p.reset();

    size_t cleaned = m.cleanup();
    REQUIRE(cleaned == 1);
    REQUIRE(m.size() == 0);

    fs::remove("test7.txt");
}

TEST_CASE("ResourceManager - удаление из кеша", "[ResourceManager]")
{
    make_file("test8.txt", "data");

    ResourceManager m;

    auto p = m.get("test8.txt", "r");
    REQUIRE(m.has("test8.txt"));

    bool removed = m.remove("test8.txt");
    REQUIRE(removed);
    REQUIRE_FALSE(m.has("test8.txt"));
    REQUIRE(p->is_open());

    fs::remove("test8.txt");
}