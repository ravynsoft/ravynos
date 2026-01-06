add_tapi_library(tapiAPIVerifier
  APIVerifier.cpp
  TAPIStructuralEquivalence.cpp

  LINK_LIBS
  clangFrontend
  clangBasic
  tapiCore
  tapiDiagnostics
  tapiFrontend
  )
