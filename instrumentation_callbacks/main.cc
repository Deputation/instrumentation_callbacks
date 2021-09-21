#include "common.h"

int main(void) {
  syms::initialize();

  if (!instrumentation::initialize()) {
    std::cout << "[+] Couldn't initialize instrumentation callbacks. "
              << std::endl;
  }

  while (true) {
    using namespace std::chrono_literals;

    // Issue a call for the callback to catch!

    // NtClose call will be caught.
    CloseHandle(reinterpret_cast<HANDLE>(0x1));

    // NtDelayExecution call will be caught.
    std::this_thread::sleep_for(1000ms);
  }

  return 0;
}