add_tapi_library(tapiDiagnostics
  Diagnostics.cpp

  DEPENDS
  TAPIDiagnosticTAPI

  LINK_LIBS
  tapiCore
  )
