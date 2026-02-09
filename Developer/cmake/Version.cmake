# ------------------------------------------------------------------------
#                        PRODUCT VERSION KNOBS
# ------------------------------------------------------------------------
set(PROD_VERSION 0.7.0)
set(PROD_FAMILY "Pre Alpha")

# Set ONE arch and ONE config
set(ARCH_CONFIGS X86_64)
set(KERNEL_CONFIGS RELEASE)

set(MACOS_VERSION_MIN 10.15)

# ------------------------------------------------------------------------
# We have not included llvm's makefiles yet and we don't want to do it
# here since this gets included multiple times, so parse them out

execute_process(OUTPUT_VARIABLE DARWIN_VERSION
    COMMAND head -1 ${ROOT_SOURCE_DIR}/Kernel/xnu/config/MasterVersion)
string(STRIP "${DARWIN_VERSION}" DARWIN_VERSION)

execute_process(OUTPUT_VARIABLE LLVM_VERSION_MAJOR
    COMMAND grep set\.LLVM_VERSION_MAJOR ${DEVEL}/Default.xctoolchain/llvm/llvm/CMakeLists.txt
)
execute_process(OUTPUT_VARIABLE LLVM_VERSION_MINOR
    COMMAND grep set\.LLVM_VERSION_MINOR ${DEVEL}/Default.xctoolchain/llvm/llvm/CMakeLists.txt
)
execute_process(OUTPUT_VARIABLE LLVM_VERSION_PATCH
    COMMAND grep set\.LLVM_VERSION_PATCH ${DEVEL}/Default.xctoolchain/llvm/llvm/CMakeLists.txt
)
string(REGEX MATCH "[0-9]+" LLVM_VERSION_MAJOR "${LLVM_VERSION_MAJOR}")
string(REGEX MATCH "[0-9]+" LLVM_VERSION_MINOR "${LLVM_VERSION_MINOR}")
string(REGEX MATCH "[0-9]+" LLVM_VERSION_PATCH "${LLVM_VERSION_PATCH}")
string(STRIP "${LLVM_VERSION_PATCH}" LLVM_VERSION_PATCH)

set(LLVM_VERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH})
set(LLVM_MAJOR ${LLVM_VERSION_MAJOR})
set(LLVM_MINOR ${LLVM_VERSION_MINOR})
set(LLVM_PATCH ${LLVM_VERSION_PATCH})

string(REGEX MATCH "^[0-9]+" PROD_MAJOR ${PROD_VERSION})
string(REGEX MATCH "\.[0-9]+\." PROD_MINOR_RAW ${PROD_VERSION})
string(REPLACE "." "" PROD_MINOR ${PROD_MINOR_RAW})
string(REGEX MATCH "[0-9]+$" PROD_PATCH ${PROD_VERSION})
string(STRIP "${PROD_PATCH}" PROD_PATCH)

string(REGEX MATCH "^[0-9]+" DARWIN_MAJOR ${DARWIN_VERSION})
string(REGEX MATCH "\.[0-9]+\." DARWIN_MINOR_RAW ${DARWIN_VERSION})
string(REPLACE "." "" DARWIN_MINOR ${DARWIN_MINOR_RAW})
string(REGEX MATCH "[0-9]+$" DARWIN_PATCH ${DARWIN_VERSION})
string(STRIP "${DARWIN_PATCH}" DARWIN_PATCH)

set(MACOSX_DEPLOYMENT_TARGET 10.15)

