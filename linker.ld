ENTRY(_start)

SECTIONS
{

	. = 0x100000;

	.text :
	{
		*(.text)
		*(.data)
		*(.rodata)
	}
	
	.bss :
	{
		*(.bss)
	}
	
	/*/DISCARD/ : 
	{
		*(.comment)
	}*/

}
