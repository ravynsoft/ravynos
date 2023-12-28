$!
$! Build procedure
$!
$! Note: you need a DCL-compatible gnu make.
$ MAKE="make381"
$ OPT=""
$!
$ if (P1 .EQS. "CONFIGURE") .OR. (P1 .EQS. "ALL")
$ then
$    set def [.bfd]
$    @configure
$    set def [-.libiberty]
$    @configure
$    set def [-.opcodes]
$    @configure
$    set def [-.binutils]
$    @configure
$    set def [-.gas]
$    @configure
$    set def [-]
$ endif
$ if (P1 .EQS. "BUILD") .OR. (P1 .EQS. "ALL")
$ then
$   set def [.bfd]
$   @build
$   set def [-.libiberty]
$   @build
$   set def [-.opcodes]
$   @build
$   set def [-.binutils]
$   @build
$   set def [-.gas]
$   @build
$   set def [-]
$ endif
$ if P1 .EQS. "MAKE"
$ then
$   ARCH=F$GETSYI("ARCH_NAME")
$   ARCH=F$EDIT(arch,"UPCASE")
$   set def [.bfd]
$   'MAKE "ARCH=''ARCH'" "OPT=''OPT'"
$   set def [-.libiberty]
$   'MAKE "ARCH=''ARCH'" "OPT=''OPT'"
$   set def [-.opcodes]
$   'MAKE "ARCH=''ARCH'" "OPT=''OPT'"
$   set def [-.binutils]
$   'MAKE "ARCH=''ARCH'" "OPT=''OPT'"
$   set def [-.gas]
$   'MAKE "ARCH=''ARCH'" "OPT=''OPT'"
$   set def [-]
$ endif
