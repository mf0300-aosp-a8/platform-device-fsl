#ifndef MF0300_UPDATER_EXEC_UTILS_H
#define MF0300_UPDATER_EXEC_UTILS_H

#include <string>
#include <vector>

/**
 * Runs external program and waits while it finishes.
 *
 * @a args[0] must be the path to executable to run.
 *
 * This function checks is target file has executable permissions,
 * and if not - sets them. so it may fail in case of read only
 * filesystem or insufficient permissions.
 *
 * @return true on success, false otherwise
 */
bool run_program(std::vector<std::string>&& args);

#endif  // MF0300_UPDATER_EXEC_UTILS_H
