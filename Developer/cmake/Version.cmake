# ------------------------------------------------------------------------
#                        PRODUCT VERSION KNOBS
# ------------------------------------------------------------------------
set(PROD_VERSION 0.6.1)
set(PROD_FAMILY "Hyperpop Hyena")

set(ARCH_CONFIGS X86_64)
set(KERNEL_CONFIGS RELEASE)

# ------------------------------------------------------------------------

string(REGEX MATCH "^[0-9]+" PROD_MAJOR ${PROD_VERSION})
string(REGEX MATCH "\.[0-9]+\." PROD_MINOR_RAW ${PROD_VERSION})
string(REPLACE "." "" PROD_MINOR ${PROD_MINOR_RAW})
string(REGEX MATCH "[0-9]+$" PROD_PATCH ${PROD_VERSION})
