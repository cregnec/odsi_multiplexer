OUTPUT_FORMAT("binary")
ENTRY(_start)

physbase = 0x00700000;

SECTIONS
{
 	.text 0x00700000:
	{
	    code = .;
	    __text_start =  ABSOLUTE(.);
	    *(.text.entry)
	    *(.text)
	    *(.text.last)
	    *(.text.*)
	    . = ALIGN(4);
		*(.rodata)
		*( .rodata.*)
 	    __text_end =  ABSOLUTE(.);
  		. = ALIGN(4096);

	}	/* Read-write data (initialised) */



    .data :
	{
 		__data_start = ABSOLUTE(.);
  		__data_lma = LOADADDR(.data);
  		__data_vma =  ABSOLUTE(.);
 		*(.data)
 		*(.data.*)
 		__data_end = ABSOLUTE(.);
 		. = ALIGN(4096);
 	}

	/* Read-write data (uninitialised) */
 	.bss :
	{
    	__bss_start =  ABSOLUTE(.);
    	*(.bss)
     	*(COMMON)
    	__bss_end =  ABSOLUTE(.);
   		. = ALIGN(4096);
 	}
  end = .; _end = .; __end = .;

}
