
// function can be coalesced and should not be dead stripped
void __attribute__ ((weak, section ("__TEXT,__text_no_strip,regular,no_dead_strip"))) foo()
{

}


// function should not be exported, can be coalesced, and should not be dead stripped
void __attribute__ ((weak, visibility("hidden"), section ("__TEXT,__text_no_strip,regular,no_dead_strip"))) hidden()
{

}

// bar should be dead stripped
void __attribute__ ((weak, section ("__DATA,__text2"))) bar()
{

}

__attribute__((constructor)) static void init() 
{
	foo();
	hidden();
}
