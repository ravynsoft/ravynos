// Same as test ARCLayouts but with MRC __weak support disabled.
/*
TEST_CONFIG MEM=arc
TEST_BUILD
    mkdir -p $T{OBJDIR}
    $C{COMPILE_NOLINK_NOMEM} -c $DIR/MRCBase.m -o $T{OBJDIR}/MRCBase.o -fno-objc-weak
    $C{COMPILE_NOLINK_NOMEM} -c $DIR/MRCARC.m  -o $T{OBJDIR}/MRCARC.o  -fno-objc-weak
    $C{COMPILE_NOLINK}       -c $DIR/ARCBase.m -o $T{OBJDIR}/ARCBase.o
    $C{COMPILE_NOLINK}       -c $DIR/ARCMRC.m  -o $T{OBJDIR}/ARCMRC.o
    $C{COMPILE} '-DNAME=\"ARCLayoutsWithoutWeak.m\"' -fobjc-arc $DIR/ARCLayouts.m -x none $T{OBJDIR}/MRCBase.o $T{OBJDIR}/MRCARC.o $T{OBJDIR}/ARCBase.o $T{OBJDIR}/ARCMRC.o -framework Foundation -o ARCLayoutsWithoutWeak.exe
END
*/
