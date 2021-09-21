#pragma once

namespace syms {
/// <summary>
/// Parser clased used to interface with the DbgHelp interface.
/// </summary>
class parser_t {
 public:
  /// <summary>
  /// Gets a function's name given its address.
  /// </summary>
  /// <param name="address">Function address (can be any number of bytes into
  /// the function.)</param> <param name="offset_into_function_output">Pointer
  /// to a variable which will receive the amount of bytes the address is into
  /// the function.</param>
  /// <returns>The function's name if successful., otherwise an error
  /// message.</returns>
  std::string get_function_sym_by_address(
      void* address, uint64_t* offset_into_function_output = nullptr);

  /// <summary>
  /// Initializes the symbol parsing interface.
  /// </summary>
  parser_t();
};

/// <summary>
/// Singleton used to let the program interface elegantly with the symbol
/// parsing interface.
/// </summary>
extern std::shared_ptr<parser_t> g_parser;

/// <summary>
/// Initializes the symbol parsing interface.
/// </summary>
void initialize();
}  // namespace syms