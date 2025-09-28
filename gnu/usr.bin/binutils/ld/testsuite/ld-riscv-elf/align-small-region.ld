MEMORY
{
 reset : ORIGIN = 0x0, LENGTH = 32
}

SECTIONS
{
 .entry :
 {
 KEEP (*(.entry))
 } > reset
}
