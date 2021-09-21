#include "common.h"

DWORD instrumentation::tls_index;

void callback(CONTEXT* ctx) {
  // by using the TLS, we can make sure to handle all syscalls coming from
  // all threads, and we can call syscalls ourselves inside the callback without
  // recursing forever.

  // Update these offsets to your windows version from ntdiff:
  // https://ntdiff.github.io/
  // /* 0x02d0 */ unsigned __int64 InstrumentationCallbackSp;
  // /* 0x02d8 */ unsigned __int64 InstrumentationCallbackPreviousPc;
  // /* 0x02e0 */ unsigned __int64 InstrumentationCallbackPreviousSp;

  // Grab the teb to extract instrumentation callback specific information.
  auto teb = reinterpret_cast<uint64_t>(NtCurrentTeb());

  // Grab and store the address we should return to.
  ctx->Rip = *reinterpret_cast<uint64_t*>(teb + 0x02d8);
  // Grab and store the stack pointer that we should restore.
  ctx->Rsp = *reinterpret_cast<uint64_t*>(teb + 0x02e0);
  // Recover original RCX.
  ctx->Rcx = ctx->R10;

  // First check if this thread is already handling a syscall, hence this coming
  // syscall is coming from inside the callback itself.
  if (instrumentation::is_thread_handling_syscall()) {
    // Abort if it's already handling a syscall, or if reading the data is not
    // successful.
    RtlRestoreContext(ctx, nullptr);
  }

  // Try setting the TLS variable to indicate that the thread is now handling a
  // syscall.
  if (!instrumentation::set_thread_handling_syscall(true)) {
    // Abort if the variable couldn't be set.
    RtlRestoreContext(ctx, nullptr);
  }

  // Grab the return address of the call.
  auto return_address = reinterpret_cast<void*>(ctx->Rip);
  // Grab the return value of the call.
  auto return_value = reinterpret_cast<void*>(ctx->Rax);
  // Variable that will hold teh amount of bytes into the function
  // the return address is located into.
  uint64_t offset_into_function;
  // Grab the function's name.
  auto function_name = syms::g_parser->get_function_sym_by_address(
      return_address, &offset_into_function);

  // Print out all the information.
  std::cout << "[rax=0x" << return_value << "] "
            << "[rip=0x" << return_address << "] "
            << "syscall returning to " << function_name << "+0x" << std::hex
            << offset_into_function << " (" << function_name << ")"
            << std::endl;

  // Unlock the thread so that it will be able to handle another syscall.
  instrumentation::set_thread_handling_syscall(false);
  // Restore the context back to the original one, continuing execution.
  RtlRestoreContext(ctx, nullptr);
}

bool* instrumentation::get_thread_data_pointer() {
  void* thread_data = nullptr;
  bool* data_pointer = nullptr;

  thread_data = TlsGetValue(instrumentation::tls_index);

  if (thread_data == nullptr) {
    thread_data = reinterpret_cast<void*>(LocalAlloc(LPTR, 256));

    if (thread_data == nullptr) {
      return nullptr;
    }

    RtlZeroMemory(thread_data, 256);


    if (!TlsSetValue(instrumentation::tls_index, thread_data)) {
      return nullptr;
    }
  }

  data_pointer = reinterpret_cast<bool*>(thread_data);

  return data_pointer;
}

bool instrumentation::set_thread_handling_syscall(bool value) {
  if (auto data_pointer = get_thread_data_pointer()) {
    *data_pointer = value;
    return true;
  }

  return false;
}

bool instrumentation::is_thread_handling_syscall() {
  if (auto data_pointer = get_thread_data_pointer()) {
    return *data_pointer;
  }

  return false;
}

bool instrumentation::initialize() {
  auto nt_dll = LoadLibrary(L"ntdll.dll");

  if (!nt_dll) {
    std::cout << "[+] Couldn't load ntdll.dll" << std::endl;

    return false;
  }

  auto nt_set_information_process =
      reinterpret_cast<instrumentation::nt_set_information_process_t>(
          GetProcAddress(nt_dll, "NtSetInformationProcess"));

  if (!nt_set_information_process) {
    std::cout << "[+] Couldn't gather address for NtSetInformationProcess"
              << std::endl;
    return false;
  }

  instrumentation::tls_index = TlsAlloc();

  if (instrumentation::tls_index == TLS_OUT_OF_INDEXES) {
    std::cout << "Couldn't allocate a TLS index" << std::endl;

    return false;
  }

  process_instrumentation_callback_info_t info;
  info.version = 0;  // x64 mode
  info.reserved = 0;
  info.callback = bridge;

  nt_set_information_process(GetCurrentProcess(),
                             static_cast<PROCESS_INFORMATION_CLASS>(0x28),
                             &info, sizeof(info));

  return true;
}