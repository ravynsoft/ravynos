add_tapi_library(tapiSDKDB
  BitcodeReader.cpp
  BitcodeWriter.cpp
  CompareConfigFileReader.cpp
  PartialSDKDB.cpp
  SDKDB.cpp

  DEPENDS
  TapiDriverOptions
  )
  