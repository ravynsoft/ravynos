
# TODO(compnerd) ensure that benchmark.h data_private.h introduction_private.h
# io_private.h layout_private.h mach_private.h private.h queue_private.h
# source_private.h are included in the source tarball

if (INSTALL_PRIVATE_HEADERS)
  install(FILES
            benchmark.h
            data_private.h
            introspection_private.h
            io_private.h
            layout_private.h
            mach_private.h
            private.h
            queue_private.h
            source_private.h
          DESTINATION
            "${INSTALL_DISPATCH_HEADERS_DIR}")
endif()
