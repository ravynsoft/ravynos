set (LLVM_LINK_COMPONENTS 
  Object
  Support
  )


add_tapi_library(tapiObjCMetadata
  ObjCMetadata.cpp

  DEPENDS
  intrinsics_gen
  )
