MEMORY
{
	RESERVED	(r)		: ORIGIN = 0x00000000, LENGTH = 32K
	INIT_RAM	(rwx)	: ORIGIN = 0x00008000, LENGTH = 32K
	RAM			(rwx)	: ORIGIN = 0x00010000, LENGTH = 64M
}

ENTRY(_start)

SECTIONS {
    /*
	 * Our init section allows us to place the bootstrap code at address 0x8000
	 *
	 * This is where the Graphics processor forces the ARM to start execution.
	 * However the interrupt vector code remains at 0x0000, and so we must copy the correct
	 * branch instructions to 0x0000 - 0x001C in order to get the processor to handle interrupts.
	 *
	 */
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

	/*
	* Next we put the data.
	*/
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
	 *	Place HEAP here???
	 **/

	/**
	 *	Stack starts at the top of the RAM, and moves down!
	 **/
	__stack = ORIGIN(RAM) + LENGTH(RAM);
}

