#include "file_locking.h"
#include <unordered_map>

namespace {
  std::unordered_map<std::string, int> file_descriptors;
  bool file_descriptor_exists(std::string file_name) {
    return file_descriptors.find(file_name) == file_descriptors.end();
  }

  int get_file_descriptor(std::string file_name) {
    if (file_descriptor_exists(file_name)) {
      file_descriptors[file_name] = open(file_name.c_str(), O_RDWR);
    }
    return file_descriptors[file_name];
  }
}

void lockFile(std::string file_name) {
  auto fd = get_file_descriptor(file_name);
  lockf(fd, F_LOCK, 0);
}

void unlockFile(std::string file_name) {
  auto fd = get_file_descriptor(file_name);
  lockf(fd, F_ULOCK, 0);
}
