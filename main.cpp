#include <cstdlib>
#include <glog/logging.h>
#include "window_manager.hpp"

using ::std::unique_ptr;

int main(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);

  unique_ptr<WindowManager> window_manager = WindowManager::Create();
  if (!window_manager) {
    LOG(ERROR) << "Failed to initialize window manager.";
    return EXIT_FAILURE;
  }

  window_manager->Run();

  return EXIT_SUCCESS;
}
