int strongsym = 123;
extern int __attribute__ ((weak, alias ("strongsym"))) weaksym1;
extern int __attribute__ ((weak, alias ("strongsym"))) weaksym2;
