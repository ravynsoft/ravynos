SECTIONS
{
    .got : { *(.got .toc) }
    .dummy : { KEEP (*(.dummy)) }
}
