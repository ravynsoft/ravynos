# VPE-LIB

VPE C library for AMD drivers

Folder Architecture
===================
```text
[root]
 |
 +-- [inc]  ## public header to external modules
 |
 +-- [src]  ##internal implementation
     |
     +-- [chip]  ## store chip specific files
     |    |
     |    +-- [vpeXX]  ## asic specific files e.g. vpe10
     |           |
     |           +-- [inc]  ## all headers for vpe[XX]
     |                 |
     |                 +-- [asic]   ## store all headers that
     |                              ## could conflict with headers in other asics
     |                              ## src file has to explicitly include the files here
     |                              ## without relying the compilation include directory path
     |
     |
     + -- [core]  ## files that share for all asics
     |    |
     |    +-- [inc]  ## define the base functions that each vpe[xx] should implement
     |
     -- [utils]  ## utility functions like fixed point or u64 calculation
          |
          +-- [inc] ## utils headers
```
