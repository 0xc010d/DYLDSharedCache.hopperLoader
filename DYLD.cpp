#include "DYLD.hpp"
#include <dyld/launch-cache/dsc_iterator.h>
#include <dyld/launch-cache/Architectures.hpp>

#define NO_ULEB

#include <dyld/launch-cache/MachOFileAbstraction.hpp>

template<typename A>
int optimize_linkedit(macho_header<typename A::P> *mh, uint64_t textOffsetInCache, const void *mapped_cache,
                      uint64_t *newSize);

namespace DYLD {

static const std::vector<std::string> kArchCacheMagicX86{
  "dyld_v1    i386"
};
static const std::vector<std::string> kArchCacheMagicX86_64{
  "dyld_v1  x86_64",
  "dyld_v1 x86_64h"
};
static const std::vector<std::string> kArchCacheMagicArm{
  "dyld_v1   armv5",
  "dyld_v1   armv6",
  "dyld_v1  armv7f",
  "dyld_v1  armv7k",
  "dyld_v1   armv7",
  "dyld_v1  armv7s"
};
static const std::vector<std::string> kArchCacheMagicArm64{
  "dyld_v1   arm64"
};

template<typename Arch>
class LibraryBuilderImp : public LibraryBuilder
{
public:
  LibraryBuilderImp(const void *data, size_t size) : m_data(data), m_size(size) { }

  virtual Library buildLibrary(const std::vector<Segment> &segments) override
  {
    //TODO: add mapped cache size checks
    typedef typename Arch::P Pointer;

    std::vector<uint8_t> data;

    // Write regular segments into the buffer
    uint64_t newSize = 0;
    uint64_t textSegmentOffset = 0;
    static const std::string textSegmentName = "__TEXT";
    for (auto it = segments.begin(); it != segments.end(); ++it) {
      if (it->name == textSegmentName) {
        textSegmentOffset = it->offset;
      }
      data.insert(data.end(), (uint8_t *)m_data + it->offset, (uint8_t *)m_data + it->offset + it->size);
      newSize += it->size;
    }

    // optimize linkedit
    uint8_t *base_ptr = &data.front();
    optimize_linkedit<Arch>((macho_header<Pointer> *)base_ptr, textSegmentOffset, m_data, &newSize);
    data.resize(newSize);

    return data;
  }

private:
  const void *m_data;
  const size_t m_size;
};

bool SharedCache::load()
{
  if (m_size < 16) return false;
  std::string magic((const char *)m_data, 15);

  //set the library builder
  if (std::find(kArchCacheMagicX86.begin(), kArchCacheMagicX86.end(), magic) != kArchCacheMagicX86.end()) {
    m_libraryBuilder.reset(new LibraryBuilderImp<x86>(m_data, m_size));
  }
  else if (std::find(kArchCacheMagicX86_64.begin(), kArchCacheMagicX86_64.end(), magic) != kArchCacheMagicX86_64.end()) {
    m_libraryBuilder.reset(new LibraryBuilderImp<x86_64>(m_data, m_size));
  }
  else if (std::find(kArchCacheMagicArm.begin(), kArchCacheMagicArm.end(), magic) != kArchCacheMagicArm.end()) {
    m_libraryBuilder.reset(new LibraryBuilderImp<arm>(m_data, m_size));
  }
  else if (std::find(kArchCacheMagicArm64.begin(), kArchCacheMagicArm64.end(), magic) != kArchCacheMagicArm64.end()) {
    m_libraryBuilder.reset(new LibraryBuilderImp<arm64>(m_data, m_size));
  }
  else {
    return false;
  }

  //get segments data
  m_segments.clear();
  void (^block)(const dyld_shared_cache_dylib_info *, const dyld_shared_cache_segment_info *) =
    ^(const dyld_shared_cache_dylib_info *dylibInfo, const dyld_shared_cache_segment_info *segmentInfo) {
      Segment segment(segmentInfo->name, segmentInfo->fileOffset, segmentInfo->fileSize);
      m_segments[dylibInfo->path].push_back(segment);
    };
  return dyld_shared_cache_iterate(m_data, (uint32_t)m_size, block) == 0;
}

std::vector<std::string> SharedCache::getPaths() const
{
  auto comparator = [](const std::string &path1, const std::string &path2) {
    auto getName = [](const std::string &path) { return path.substr(path.find_last_of("/") + 1); };
    return getName(path1) < getName(path2);
  };
  std::set<std::string, decltype(comparator)> paths(comparator);
  for (auto &it : m_segments) {
    paths.insert(it.first);
  }
  return {paths.begin(), paths.end()};
}

Library SharedCache::getLibrary(const std::string &path)
{
  return m_libraryBuilder->buildLibrary(m_segments[path]);
}

}
