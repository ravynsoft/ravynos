void
__cdecl
lib2foocdecl(int junk1, int* junk2)
{
    *junk2 = junk1;
}

void
__stdcall
lib2foostdcall(int junk1, int* junk2)
{
    *junk2 = junk1;
}

void
__fastcall
lib2foofastcall(int junk1, int* junk2)
{
    *junk2 = junk1;
}
