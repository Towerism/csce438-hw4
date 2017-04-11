#include <unistd.h>
#include <fcntl.h>

#include <string>

void lockFile(std::string file_name);
void unlockFile(std::string file_name);
