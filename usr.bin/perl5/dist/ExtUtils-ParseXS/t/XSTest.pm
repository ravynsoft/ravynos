package XSTest;

require DynaLoader;
@ISA = qw(Exporter DynaLoader);
$VERSION = '0.01';
bootstrap XSTest $VERSION;

1;
