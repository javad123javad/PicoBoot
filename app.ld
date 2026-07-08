
MEMORY
{
	FLASH (rx) : ORIGIN = 0x00001000, LENGTH = 0x00080000
	RAM (rwx) : ORIGIN  = 0x20000000, LENGTH = 0x00020000
	CCRAM(rwx) : ORIGIN = 0x10000000, LENGTH = 0x00010000
}

SECTIONS
{
	.text :	{
		_start_text = .;
		KEEP(*(.isr_vector))
		KEEP(*(.text*))
		*(.rodata*)
		. = ALIGN(4);
		end_text = .;
	} > FLASH
	
	_stored_ccmram = .;
	.ccmram : AT (_stored_ccmram)
	{
		_start_ccmram = .;
		*(.ccmram)
		. = ALIGN(4);
		_end_ccmram = .;
	} > CCRAM

	_stored_data = .;
	
	.data : AT (_stored_data)
	{
		_start_data = .;
		KEEP(*(.data*))
		. = ALIGN(4);
		_end_data = .;
	} > RAM

	.bss :{
		_start_bss = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_end_bss = .;
		_end = .;
	} > RAM
}

END_STACK = ORIGIN(RAM) + LENGTH(RAM);

