
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include <fat.h>

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main ( void ) 
{
//---------------------------------------------------------------------------------
	
	
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	consoleDemoInit();
	
	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	////iprintf("\x1b[10;10HHello World!\n");
	
	
	// Initialize FAT
	//*(unsigned int *)(0x0E000000) = fatInitDefault ();
	
	bool ggggg = fatInitDefault ();
	
	FILE* test = fopen ( "fat:/GGGGG.txt", "wt" );
	fclose (test);
	
	iprintf ( "\x1b[10;10HHello World!\n" );
	
	for (;;)
	{
		VBlankIntrWait();
	}
}


