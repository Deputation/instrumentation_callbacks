#pragma once

namespace instrumentation {
/// <summary>
/// Structure describing the syscall NtSetInformationProcess, its return value
/// and its arguments.
/// </summary>
using nt_set_information_process_t = NTSTATUS(NTAPI*)(HANDLE,
                                                      PROCESS_INFORMATION_CLASS,
                                                      PVOID, ULONG);
/// <summary>
/// Structure describing the callback the system will call.
/// </summary>
using bridge_function_t = void (*)();

/// <summary>
/// System structure used to describe an instrumentation callback, will be sent
/// to the NtSetInformationProcess syscall.
/// </summary>
struct process_instrumentation_callback_info_t {
  uint32_t version;
  uint32_t reserved;
  bridge_function_t callback;
};

/// <summary>
/// Structure representing registers pushed into the stack by the instrumentation callback.
/// </summary>
struct registers_t {
  void* rax;
  void* rcx;
  void* rbx;
  void* rbp;
  void* rdi;
  void* rsi;
  void* rsp;
  void* r10;
  void* r11;
  void* r12;
  void* r13;
  void* r14;
  void* r15;
};

/// <summary>
/// TLS index used to store thread specific information.
/// </summary>
extern DWORD tls_index;

/// <summary>
/// Get a pointer to the thread specific information we allocated (a boolean
/// telling us whether the current thread is handling a syscall or not.
/// </summary>
/// <returns>A pointer to the boolean describing whether or not the current
/// thread is handling a syscall.</returns>
bool* get_thread_data_pointer();

/// <summary>
/// Gets a pointer to the thread specific data describing whether or not the
/// current thread is handling a syscall, and sets it to the value defined in parameters.
/// </summary>
/// <param name="value">Value to set the syscall "lock" to.</param>
/// <returns>True if the operation was successful, false if not.</returns>
bool set_thread_handling_syscall(bool value);

/// <summary>
/// Gets a pointer to the thread specific data describing whether or not the
/// curreent thread is handling a syscall, dereferences it and returns its
/// value.
/// </summary>
/// <returns>Ture if the variable was set to true, false if it wasn't or there
/// was an error.</returns>
bool is_thread_handling_syscall();

/// <summary>
/// Initializes the TLS index and places the instrumentation callback.
/// </summary>
/// <returns>True if initialization was successful, false if not.</returns>
bool initialize();
}  // namespace instrumentation

/// <summary>
/// "Bridge" function used to call the instrumentationc allback and setting up
/// the stack properly.
/// </summary>
extern "C" void bridge();

/// <summary>
/// Actual instrumentation callback which will receive syscall. Extern'd so that
/// it may be called directly from the .asm file.
/// </summary>
/// <param name="ctx">syscall context.</param>
extern "C" void callback(CONTEXT* ctx);