
extern void LibInit(void);

extern void OutputString(char *String);



char * gString = "\nHello world via mtoc - reloc\n";       

int gWhyAreUninitializedGlobalsBad;
       

void _ModuleEntryPoint(void)
{ 
  //__asm__ __volatile__ ("int $3");  

  LibInit ();
  gWhyAreUninitializedGlobalsBad =1;
  OutputString(gString);
}

