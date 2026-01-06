set(LLVM_LINK_COMPONENTS
  Support
  Core
  )

add_tapi_library(tapiFrontend
  APIVisitor.cpp
  Frontend.cpp
  FrontendContext.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangFrontend
  clangIndex
  tapiDiagnostics
  )
