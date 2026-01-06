extern char text_start[] __asm("segment$start$__TEXT");
extern char text_end[] __asm("segment$end$__TEXT");
extern char text_text_start[] __asm("section$start$__TEXT$__text");

void* a = &text_start;
void* b = &text_end;
void* c = &text_text_start;

void start(void)
{
}

