
function(dispatch_set_linker target)
  if(USE_GOLD_LINKER)
    set_property(TARGET ${target}
                 APPEND_STRING
                 PROPERTY LINK_FLAGS
                   -fuse-ld=gold)
  endif()
  if(USE_LLD_LINKER)
    set_property(TARGET ${target}
                 APPEND_STRING
                 PROPERTY LINK_FLAGS
                   -fuse-ld=lld)
  endif()
endfunction()
