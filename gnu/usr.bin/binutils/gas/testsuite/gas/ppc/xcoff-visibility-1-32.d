#as: -a32
#source: xcoff-visibility-1.s
#objdump: -t
#name: XCOFF Visibility 1 (32 bit)

.*

SYMBOL TABLE:
.*
.*
.*
.*
\[  4\].*\(ty    0\).*l_novisibility
.*
\[  6\].*\(ty 1000\).*l_internal
.*
\[  8\].*\(ty 2000\).*l_hidden
.*
\[ 10\].*\(ty 3000\).*l_protected
.*
\[ 12\].*\(ty 4000\).*l_exported
.*
\[ 14\].*\(ty 1000\).*l_dual
.*
\[ 16\].*\(ty    0\).*globl_novisibility
.*
\[ 18\].*\(ty 1000\).*globl_internal
.*
\[ 20\].*\(ty 2000\).*globl_hidden
.*
\[ 22\].*\(ty 3000\).*globl_protected
.*
\[ 24\].*\(ty 4000\).*globl_exported
.*
\[ 26\].*\(ty 1000\).*globl_dual
.*
\[ 28\].*\(ty    0\).*weak_novisibility
.*
\[ 30\].*\(ty 1000\).*weak_internal
.*
\[ 32\].*\(ty 2000\).*weak_hidden
.*
\[ 34\].*\(ty 3000\).*weak_protected
.*
\[ 36\].*\(ty 4000\).*weak_exported
.*
\[ 38\].*\(ty 1000\).*weak_dual
.*
\[ 40\].*\(ty    0\).*comm_novisibility
.*
\[ 42\].*\(ty 1000\).*comm_internal
.*
\[ 44\].*\(ty 2000\).*comm_hidden
.*
\[ 46\].*\(ty 3000\).*comm_protected
.*
\[ 48\].*\(ty 4000\).*comm_exported
.*
\[ 50\].*\(ty    0\).*extern_novisibility
.*
\[ 52\].*\(ty 1000\).*extern_internal
.*
\[ 54\].*\(ty 2000\).*extern_hidden
.*
\[ 56\].*\(ty 3000\).*extern_protected
.*
\[ 58\].*\(ty 4000\).*extern_exported
.*
\[ 60\].*\(ty 1000\).*extern_dual
.*
