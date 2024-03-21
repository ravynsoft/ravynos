SECTIONS
{
    .text : { *(.text) }

    __foo0_start = .;
    .foo0 : { *(.foo0.*) }
    __foo0_end = .;

    __foo1_start = .;
    .foo1 : { KEEP(*(.foo1.*)) }
    __foo1_end = .;

    .foo2 : {
        __foo2_start = .;
        KEEP(*(.foo2.*))
        __foo2_end = .;
    }
    /DISCARD/ : { *(*) }
}

ASSERT (__foo1_start < __foo1_end, "foo1 not KEPT");
ASSERT ((__foo1_end - __foo1_start) == (__foo2_end - __foo2_start),"foo2 not KEPT");
