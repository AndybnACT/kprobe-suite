/* placing my named section at given address: */
SECTIONS
{
	.data :  {
		*(.data)
		. = ALIGN(8);
__MYPROBE_SECTION_START = .;  /* create symbol for start of section */
		KEEP(*(.my_probes)) /* keep my variable even if not referenced */
__MYPROBE_SECTION_END = .; /* create symbol for end of section */
	}
}
