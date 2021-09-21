#include "common.h"

std::shared_ptr<syms::parser_t> syms::g_parser;

std::string syms::parser_t::get_function_sym_by_address(
    void* address, uint64_t* offset_into_function_output) {
  auto buffer = std::malloc(sizeof(SYMBOL_INFO) + MAX_SYM_NAME);

  if (!buffer) {
    return "Couldn't retrieve function name.";
  }

  RtlZeroMemory(buffer, sizeof(buffer));

  auto symbol_information = reinterpret_cast<PSYMBOL_INFO>(buffer);
  symbol_information->SizeOfStruct = sizeof(SYMBOL_INFO);
  symbol_information->MaxNameLen = MAX_SYM_NAME;

  DWORD64 offset_into_function = 0;

  auto result = SymFromAddr(reinterpret_cast<HANDLE>(-1),
                            reinterpret_cast<DWORD64>(address),
                            &offset_into_function, symbol_information);

  if (!result) {
    std::free(buffer);
    return "Couldn't retrieve function name.";
  }

  if (offset_into_function_output) {
    *offset_into_function_output = offset_into_function;
  }

  auto built_string = std::string(symbol_information->Name);
  
  std::free(buffer);

  return built_string;
}

syms::parser_t::parser_t() {
  SymSetOptions(SYMOPT_UNDNAME);
  SymInitialize(reinterpret_cast<HANDLE>(-1), nullptr, true);
}

void syms::initialize() { g_parser = std::make_shared<parser_t>(); }