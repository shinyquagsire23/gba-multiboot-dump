
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>

#include <fat.h>

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------

u16 xfer16(u16 tosend) 
{
    //Wait for exchange to start
    REG_SIOMLT_SEND = tosend;
    while((REG_SIOCNT & 0x80) == 0){}

	//Master sends data, we read
	while((REG_RCNT & 0x1) == 0){}
	u16 ret = REG_SIOMULTI0; //Get data
	
	//Wait until all slaves done
	while((REG_SIOCNT & 0x80) == 1){}
	REG_SIOMLT_SEND = 0xa5df;
	
	return ret;
}

u16 xfer16_end() 
{
    //Wait for exchange to start
    REG_SIOMLT_SEND = 0xFF;
    while((REG_SIOCNT & 0x80) == 0){}

	//Master sends data, we read
	while((REG_RCNT & 0x1) == 0){}
	u16 ret = REG_SIOMULTI0; //Get data
	
	//Wait until all slaves done
	while((REG_SIOCNT & 0x80) == 1){}
	
	return ret;
}

void decrypt(void *data, u32 imagesize, u8 pp) 
{
	u32 *p=(u32*)(data + 0xC0);
	u32 *p_enc =(u32*)0x20000c0;
	u32 key_const = 0x6F646573;
	u32 key = (0xFFFFEA00 | pp);//0x909B9A8C;
	u32 type_key = 0x6465646F; //JOYBUS = 0x20796220, Normal = 0x43202F2F, Multi = 0x6465646F
	u32 word;

    //iprintf("%x\n", (*p)^(-(u32)p)^(type_key));
    //p++;
    //iprintf("%x\n", (*p)^(-(u32)p)^(type_key));
    //p--;

	imagesize=(imagesize-0xc0)/4;
	do {
		key=(key*key_const)+1;
		word=(*p)^(key)^(-(u32)p_enc)^(type_key);
		*p=word;
		p++;
		p_enc++;
	} while(--imagesize);

	//p=(u32*)0x20000000;    //fixup entry point
	//p[0]=0xea000036;
}

int main ( void ) 
{
//---------------------------------------------------------------------------------
	
	
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	void *store = malloc(0x10000);
	irqInit();
	irqEnable(IRQ_VBLANK);
	int i = 0;
	int rec = 0;
	
	consoleDemoInit();
	REG_IE=REG_IE|0x0080;	 	//serial interrupt enable
	REG_RCNT = 0;
	REG_SIOCNT = 0x2003;
	
	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	////iprintf("\x1b[10;10HHello World!\n");
	
	
	// Initialize FAT
	//*(unsigned int *)(0x0E000000) = fatInitDefault ();
	
	bool hasFat = fatInitDefault ();
	
	iprintf ("Waiting for link...\n");
	while(1)
	{
	    u16 rec = xfer16(0x0);
	    //iprintf("%x\n", rec);
	    if(rec == 0x6200)
	        break;
	}
	
	iprintf ("exchange master/slave\n");
	while(rec != 0x61)
	{
	    rec = xfer16(0x7202) >> 8;
	    //if(rec != 0x62)
  	        //printf("%x\n", rec);
	}
	
	u32 addr = 0;
	iprintf ("0x60 count\n");
	//Count down from 0x60
	for(i = 0x60; i >= 0; i--)
	{
	    u16 rec = xfer16((i << 8) | 0x2);
	    //printf("%x for %x\n", (i << 8) | 0x2, rec);
	    *(u16*)(store + addr) = rec;
	    addr += 2;
	}
	
	iprintf ("exchange master/slave\n");
	while(rec != 0x63)
	{
	    rec = (xfer16(0x7202) >> 8);
	    //if(rec != 0x62 && rec != 0x61 && rec)
    	    //printf("%x\n", rec);
	} 
	u8 pp = xfer16(0x73EA);
	printf("pp %x\n", pp); //Rando data
	printf("%x\n", xfer16(0x73B7)); //Rando data
	u16 length = xfer16(0x73a4); //Data length
	
	addr = 0x00c0;
	int j;
	for(i = 0; i <= ((length*4)+0x190-0xc0)/2; i++)
	{
        rec = xfer16(addr);
	    *(u16*)(store + addr) = rec;
   	    addr += 2;
	}
	
	for(i = 0; i < 12; i++)
	{
	    xfer16(0x74);
	}
	xfer16(0x75);
	xfer16(0x75);
	u16 crc = xfer16(0x045c); //Exchange CRCs. TODO
	xfer16_end();
	
	printf("read: %x\n", addr-2);
	printf("length: %x\n", length);
	printf("mblength: %x\n", (length*4)+0x190);
	printf("crc: %x\n", crc);
	
	decrypt(store, addr-2, pp);

	if(hasFat)
	{
	    iprintf("Writing file...\n");
	    FILE* test = fopen ( "fat:/mbdump.bin", "wt" );
	    fwrite(store, 1, addr-2, test);
	    fclose (test);
	}
	
	iprintf ("Waiting forever...\n");
	for (;;)
	{
	    xfer16_end();
		VBlankIntrWait();
	}
}


