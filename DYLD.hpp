#pragma once

#include <stddef.h>
#include <map>
#include <vector>
#include <string>
#include <set>

namespace DYLD {

struct Segment;
struct Library;
class LibraryBuilder;

class SharedCache
{
public:
  SharedCache(const void *data, size_t size) : m_data(data), m_size(size) { }

  bool load();

  std::vector<std::string> getPaths() const;

  Library getLibrary(const std::string &name);

private:
  const void *m_data;
  const size_t m_size;

  std::map<std::string, std::vector<Segment>> m_segments;
  std::unique_ptr<LibraryBuilder> m_libraryBuilder;
};

struct Library
{
  Library(const std::vector<uint8_t> &data) : data(data) { }

  std::vector<uint8_t> data;
};

struct Segment
{
  Segment(const std::string &name, uint64_t offset, uint64_t size) : name(name), offset(offset), size(size) { }

  const std::string name;
  const uint64_t offset;
  const uint64_t size;
};

class LibraryBuilder
{
public:
  virtual Library buildLibrary(const std::vector<Segment> &segments) = 0;
};

}
