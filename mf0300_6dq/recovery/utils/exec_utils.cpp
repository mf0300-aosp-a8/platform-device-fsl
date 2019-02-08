#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "exec_utils.h"


static bool is_executable(const std::string& filename) {
  struct stat sb;
  if (stat(filename.c_str(), &sb) == 0) {
    int exec_perms = S_IXUSR | S_IXGRP | S_IXOTH;
    return S_ISREG(sb.st_mode) && (sb.st_mode & 0777 & exec_perms) != 0;
  }
  return false;
}


static bool make_executable(const std::string& filename) {
  return chmod(filename.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
}


bool run_program(std::vector<std::string>&& args) {
  if (!is_executable(args[0]) && !make_executable(args[0])) {
    return false;
  }

  char* args2[args.size() + 1];
  for (size_t i = 0; i < args.size(); i++) {
    args2[i] = &args[i][0];
  }
  args2[args.size()] = nullptr;

  pid_t child = fork();
  if (child == 0) {
    execv(args2[0], args2);
    _exit(EXIT_FAILURE);
  }

  int status;
  waitpid(child, &status, 0);

  return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
}
