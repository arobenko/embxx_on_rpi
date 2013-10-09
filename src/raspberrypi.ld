MEMORY
{
	RESERVED	(r)		: ORIGIN = 0x00000000, LENGTH = 32K
	INIT_RAM	(rwx)	: ORIGIN = 0x00008000, LENGTH = 32K
	RAM			(rwx)	: ORIGIN = 0x00010000, LENGTH = 64M
}

ENTRY(_start)

SECTIONS {
	.start : {
		KEEP(*(.init))
	} > INIT_RAM = 0

	/**
 	 *	This is the main code section.
	 *
	 **/
	.text : {
		*(.text)
	} > RAM
	
	.data : {
		*(.data)
	} > RAM

	.bss :
	{
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		__bss_end = .;
	} > RAM

	/**
	 *	Currently no heap
	 **/

	/**
	 *	Stack starts at the top of the RAM, and moves down!
	 **/
	__stack = ORIGIN(RAM) + LENGTH(RAM);
}

