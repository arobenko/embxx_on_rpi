MEMORY
{
    RESERVED (r)        : ORIGIN = 0x00000000, LENGTH = 32K
    RAM (rwx)           : ORIGIN = 0x00008000, LENGTH = 64M
}

ENTRY(_entry)

SECTIONS {
    .start : {
        KEEP(*(.init))
        KEEP(*(.fini))
    } > RAM = 0

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
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        __bss_end__ = .;
    } > RAM
	
    .init.array :
    {
        __init_array_start = .;
        *(.init_array)
        *(.init_array.*)
        __init_array_end = .;
    } > RAM
    
    /* .ARM.exidx is required for exception handling. It is required only
        for "test_cpp" applications that link in stdlib */
    .ARM.exidx : 
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } >RAM 

    /**
     *	Currently no heap
     **/

    /**
     *	Stack starts at the top of the RAM, and moves down!
     **/
    __stack = ORIGIN(RAM) + LENGTH(RAM);
}

