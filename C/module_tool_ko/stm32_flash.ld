
/* Entry Point */
ENTRY(main)

/* Define output sections */
SECTIONS
{
	. = 0x00;
	.text : { 	*(.text) 
			*(.test.*)
		}
	.rodata : {	*(.rodata) 
			*(.rodata.*)
		}
	.data : {	*(.data) 
			*(.data.*)
		}
	.bss :  {	*(.bss) 
			*(.bss.*)
			*(COMMON)
		}

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
