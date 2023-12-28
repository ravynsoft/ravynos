extern void abort (void);

void
__cdecl
lib2foocdecl(int junk1, int* junk2);

void
__stdcall
lib2foostdcall(int junk1, int* junk2);

void
__fastcall
lib2foofastcall(int junk1, int* junk2);

void
__cdecl
lib1foocdecl(int junk1, int* junk2)
{
    lib2foocdecl(junk1, junk2);
}

void
__stdcall
lib1foostdcall(int junk1, int* junk2)
{
    lib2foostdcall(junk1, junk2);
}

void
__fastcall
lib1foofastcall(int junk1, int* junk2)
{
    lib2foofastcall(junk1, junk2);
}

int main()
{
  int junk[3];
  lib1foofastcall (1, &junk[0]);
  lib1foostdcall (2, &junk[1]);
  lib1foocdecl (3, &junk[2]);
  if (junk[1] != 2 || junk[0] != 1 || junk[2] != 3)
    abort ();

  return 0;
}

