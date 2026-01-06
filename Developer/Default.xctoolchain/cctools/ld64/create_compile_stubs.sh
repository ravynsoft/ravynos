echo 'static const char *compile_stubs = ' > compile_stubs.h
cat compile_stubs | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$/\\n"/' >> compile_stubs.h
echo ';' >> compile_stubs.h
