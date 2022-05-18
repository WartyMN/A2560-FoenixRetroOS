RAMSTART = 0x00020000;
RAMSIZE  = 0x00300000;
STACKLEN = 0x800;

MEMORY
{
 ram : org = RAMSTART, len = RAMSIZE - STACKLEN
}

SECTIONS
{
  text ALIGN(0x04) : {*(CODE)} >ram
  .dtors ALIGN(0x04) : { *(.dtors) } > ram
  .ctors ALIGN(0x04) : { *(.ctors) } > ram
  rodata : {*(RODATA)} >ram
  data: {*(DATA)} >ram
  bss (NOLOAD): {*(BSS)} >ram

  ___heap = ADDR(bss) + SIZEOF(bss);
  ___heapend = RAMSTART + RAMSIZE - STACKLEN;


  ___BSSSTART = ADDR(bss);
  ___BSSSIZE  = SIZEOF(bss);

  ___STACK = RAMSTART + RAMSIZE;
}

