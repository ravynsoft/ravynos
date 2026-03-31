/* 
TEST_BUILD
$DIR/headers.sh '$C{TESTINCLUDEDIR}' '$C{TESTLOCALINCLUDEDIR}' '$C{COMPILE_C}' '$C{COMPILE_CXX}' '$C{COMPILE_M}' '$C{COMPILE_MM}' '$VERBOSE'
$C{COMPILE_C} $DIR/headers.c -o headers.exe
END

allow `sh -x` output from headers.sh
TEST_BUILD_OUTPUT
(\+ .*\n)*(\+ .*)?done
END
 */


#include "test.h"

int main()
{
    succeed(__FILE__);
}
