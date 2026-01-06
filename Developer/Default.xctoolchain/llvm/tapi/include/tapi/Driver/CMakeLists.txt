set(LLVM_TARGET_DEFINITIONS TAPIOptions.td)
tablegen(LLVM TAPIOptions.inc -gen-opt-parser-defs)
add_public_tablegen_target(TapiDriverOptions)
