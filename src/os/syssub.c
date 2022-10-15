#include "os/syssub.h"

#include <malloc.h>
#include <libpad.h>

#include "os/system.h"

#include "macro.h"

// Malloc report
struct MallocReport
{
	void *adr;
	size_t size;
};
static struct MallocReport usr_malloc_str[256];

// Pad state
struct SysPadData
{
	unsigned char status;
	unsigned char tag;
	unsigned char held[2];
	unsigned char a[4];
	unsigned char p[12];
};

struct SysPad
{
	unsigned char data[32];
	unsigned int id;
	unsigned int status;
	unsigned char act_align[6];
	short unk6;
	char act[6];
	short unk8;
	unsigned int unk9;
	unsigned int unk10;
};

struct SysPad sysPad[2];

static char pad_dma_buf[2][0x100] MACRO_AL64;

// Syssub functions
void WorkClear(void *ptr, size_t size)
{
	int i;
	char *p = (char*)ptr;
	for (i = ((int)size - 1); i >= size; i--)
		*p++ = 0;
}

void GPadInit(void)
{
	int i;

	// Clear pad states
	WorkClear(pad, sizeof(pad));
	WorkClear(sysPad, sizeof(sysPad));

	// Initialize pads
	scePadInit(0);
	for (i = 0; i < 2; i++)
		if (scePadPortOpen(i, 0, pad_dma_buf[i]) == 0)
			printf("ERROR: scePadPortOpen[%d]\n", i);
}

void GPadExit(void)
{
	int i;

	// Close pads
	for (i = 0; i < 2; i++)
		scePadPortClose(i, 0);
	scePadEnd();
}

void GPadSysRead(void)
{
	int i;

	// Read pads
	struct SysPad *sys_pad = sysPad;
	for (i = 0; i < 2; i++, sys_pad)
	{
		// Get pad state
		int state = scePadGetState(i, 0);
		WorkClear(sysPad[i].data, sizeof(sys_pad->data));
		
		switch (state)
		{
			case scePadStateDiscon:
				// Pad not connected
				WorkClear(&sysPad[i], sizeof(sysPad[i]));
				break;
			case scePadStateFindCTP1:
			case scePadStateStable:
				switch (sys_pad->status)
				{
					case 0:
					{
						// Get ID
						int id = scePadInfoMode(i, 0, InfoModeCurID, 0);
						int exid = scePadInfoMode(i, 0, InfoModeCurExID, 0);
						if (exid >= 0)
							id = exid;
						
						if (id == 0)
						{
							// Pad bad ID
							WorkClear(&sysPad[i], sizeof(sysPad[i]));
						}
						else
						{
							// Set pad status
							sys_pad->id = id;
							switch (id)
							{
								case 4:
									sys_pad->status = 40;
									break;
								case 7:
									sys_pad->status = 70;
									break;
								default:
									WorkClear(&sysPad[i], sizeof(sysPad[i]));
									break;
							}
						}
						break;
					}

					case 40:
					{
						// Check if it's an analog controller
						if (scePadInfoMode(i, 0, InfoModeIdTable, -1) == 0)
						{
							sys_pad->status = 99;
							break;
						}
						sys_pad->status++;
					}
				// Fallthrough
					case 41:
					{
						// Switch to analog mode
						if (scePadSetMainMode(i, 0, 1, 0) == 1)
							sys_pad->status++;
						break;
					}
					case 42:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sys_pad->status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sys_pad->status = 0;
						break;
					}

					case 70:
					{
						// Check for Dualshock
						if (scePadInfoMode(i, 0, InfoModeIdTable, -1) == 0)
						{
							sys_pad->status = 99;
							break;
						}
						if (scePadSetMainMode(i, 0, 1, 3) == 1)
							sys_pad->status++;
						break;
					}
					case 71:
					case 77:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sys_pad->status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sys_pad->status = 80;
						break;
					}
					case 72:
					{
						if (scePadInfoPressMode(i, 0) == 1)
						{
							sys_pad->status = 80;
							break;
						}
						sys_pad->status = 76;
						break;
					}
					case 76:
					{
						if (scePadEnterPressMode(i, 0) == 1)
							sys_pad->status++;
						break;
					}

					case 80:
					{
						// Get actuator information
						if (scePadInfoAct(i, 0, -1, 0) == 0)
							sys_pad->status = 99;
						
						// Set actuator information
						sys_pad->act_align[1] = 1;
						sys_pad->act_align[5] = 0xFF;
						sys_pad->act_align[0] = 0;
						sys_pad->act_align[2] = 0xFF;
						sys_pad->act_align[3] = 0xFF;
						sys_pad->act_align[4] = 0xFF;
						if (scePadSetActAlign(i, 0, sys_pad->act_align) == 0)
							break;
						
						sys_pad->status++;
						break;
					}
					case 81:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sys_pad->status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sys_pad->status = 99;
						break;
					}
				}
				break;
			default:
				break;
		}
	}
}

void padMakeData(struct Pad *pad, unsigned short held)
{
	unsigned short last_held = pad->held;
	pad->held = held;
	unsigned short change = last_held ^ held;
	pad->change = change;
	pad->release = change & ~held;
	pad->press = held & change;
}

void pad0Clear(struct Pad *pad)
{
	unsigned short last_held = pad->held;
	pad->held = 0;
	pad->change = last_held;
	pad->release = last_held;
	pad->press = 0;
}

void padOneOffBitCLear(struct Pad *pad)
{
	pad->release = 0;
	pad->press = 0;
}

void padNormalRead(struct Pad *pad, struct SysPadData *data)
{
	padMakeData(pad, ~(((unsigned short)data->held[0] << 8) | ((unsigned short)data->held[1] << 0)));
}

void padAnaRead(struct Pad *pad, struct SysPadData *data)
{
	int i;
	for (i = 0; i < 4; i++)
		pad->a[i] = data->a[i];
}

void padAnaRead0Clear(struct Pad *pad)
{
	int i;
	for (i = 0; i < 4; i++)
		pad->a[i] = 0x80;
}

void padPrsRead(struct Pad *pad, struct SysPadData *data)
{
	int i;
	for (i = 0; i < 12; i++)
		pad->p[i] = data->p[i];
}

void padPrsRead0Clear(struct Pad *pad)
{
	int i;
	for (i = 0; i < 4; i++)
		pad->a[i] = 0;
}

void padPrsTreate(struct Pad *pad)
{
	// Define pressure buttons
	const unsigned short prs_buttons[12] = {
		PAD_RIGHT,
		PAD_LEFT,
		PAD_UP,
		PAD_DOWN,
		PAD_TRIANGLE,
		PAD_CIRCLE,
		PAD_CROSS,
		PAD_SQUARE,
		PAD_L1,
		PAD_R1,
		PAD_L2,
		PAD_R2
	};

	// Set pressure buttons
	int i;

	unsigned short held = pad->held;
	for (i = 0; i < 12; i++)
	{
		if (pad->held & prs_buttons[i])
		{
			if (pad->p[i])
				continue;
			pad->p[i] = 1;
		}
		else
		{
			if (!pad->p[i])
				continue;
			pad->p[i] = 0;
		}
		held = pad->held;
	}
}

void padActSet(struct Pad *pad, struct SysPad *sys_pad)
{
	if (pad == NULL)
	{
		sys_pad->act[1] = 0;
		sys_pad->act[0] = 0;
		return;
	}

	sys_pad->act[0] = pad->act[0];
	sys_pad->act[1] = pad->act[1];
}

void padActClear(struct Pad *pad)
{
	pad->act[1] = 0;
	pad->act[0] = 0;
}

void padAnaMixPad(struct Pad *pad)
{
	unsigned char a;

	unsigned short last_aheld = pad->aheld;
	pad->aheld = 0;

	// X axis mix
	a = pad->a[PAD_LX];
	if (a < 0x40)
		pad->aheld |= PAD_LEFT;
	if (a > 0xC0)
		pad->aheld |= PAD_RIGHT;

	// Y axis mix
	a = pad->a[PAD_LY];
	if (a < 0x40)
		pad->aheld |= PAD_UP;
	if (a > 0xC0)
		pad->aheld |= PAD_DOWN;
	
	// Set press
	pad->apress = pad->aheld & (last_aheld ^ pad->aheld);
}

void GPadRead(struct Pad *pad)
{
	int i;
	for (i = 0; i < 2; i++, pad++)
	{
		struct SysPad *sys_pad = &sysPad[i];

		// Get pad status and tag
		unsigned char tag;
		if (sys_pad->data[0] == 0)
		{
			tag = sys_pad->data[1];
		}
		else
		{
			pad0Clear(pad);
			tag = sys_pad->data[1];
		}

		// Read pad
		if ((tag & 0xF0) == 0x40)
		{
			// Standard controller
			padNormalRead(pad, (struct SysPadData*)sys_pad);
			padAnaRead0Clear(pad);
			padPrsRead0Clear(pad);
			pad->status = 0x40;
			padActSet(NULL, sys_pad);
		}
		else if ((tag & 0xF0) == 0x70)
		{
			// Dualshock
			padNormalRead(pad, (struct SysPadData*)sys_pad);
			padAnaRead(pad, (struct SysPadData*)sys_pad);
			
			if (sys_pad->data[1] == 0x79)
			{
				// Dualshock with pressure data
				padPrsRead(pad, (struct SysPadData*)sys_pad);
				pad->status = 0x79;
				padPrsTreate(pad);
			}
			else
			{
				// No pressure data
				padPrsRead0Clear(pad);
				pad->status = 0x70;
			}
		}
		else
		{
			// Bad tag
			pad0Clear(pad);
			padAnaRead0Clear(pad);
			padPrsRead0Clear(pad);
		}

		padAnaMixPad(pad);
		padActClear(pad);
	}
}

void SetBackColor(int r, int g, int b)
{
	// Set db clear color
	DBufDc.clear0.rgbaq.R = r;
	DBufDc.clear0.rgbaq.G = g;
	DBufDc.clear0.rgbaq.B = b;

	DBufDc.clear1.rgbaq = DBufDc.clear0.rgbaq;
}

int randMakeMax(int max)
{
	return ((rand() & 0x7FFF) * max) >> 15;
}

char *ByteStringSub(char *s, int v)
{
	const char *format;
	if (v < 1000)
	{
		format = "%u";
	}
	else
	{
		char *ns = ByteStringSub(s, v / 1000);
		format = "%03u";
		ns[0] = ',';
		s = ns + 1;
	}
	return s + sprintf(s, format, v % 1000);
}

char *ByteString(int v)
{
	static char s[256];
	char *se = ByteStringSub(s, v);
	
	static const char *sbyte = "(byte)";
	memcpy(se, sbyte, 6);
}

void ReportHeapUsage(void)
{
	// Get malloc info
	struct mallinfo malloc_info = mallinfo();

	// Print malloc info
	printf("_________________________________________\n");
	printf("total space allocated from system = %s\n", ByteString(malloc_info.arena));
	printf("total space in mmapped regions = %s\n", ByteString(malloc_info.hblkhd));
	printf("total allocated space = %s\n", ByteString(malloc_info.uordblks));
	printf("total non-inuse space = %s\n", ByteString(malloc_info.fordblks));
	printf("number of non-inuse chunks = %d\n", malloc_info.ordblks);
	printf("number of mmapped regions = %d\n", malloc_info.hblks);
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}

void usrMallcInit(void)
{
	// Clear malloc reports
	WorkClear(&usr_malloc_str, sizeof(usr_malloc_str));
}

void *usrMalloc(size_t size)
{
	// Allocate memory
	void *adr = malloc(size);

	if (adr != NULL)
	{
		// Search for free report block
		int i;
		for (i = 0; i < MACRO_COUNTOF(usr_malloc_str); i++)
		{
			if (usr_malloc_str[i].adr == NULL)
			{
				usr_malloc_str[i].adr = adr;
				usr_malloc_str[i].size = size;
				return adr;
			}
		}

		// Free allocated memory
		free(adr);
	}

	// Malloc failed
	printf("malloc is NG\n");
	return NULL;
}

void usrFree(void *adr)
{
	// Search for report block
	int i;
	for (i = 0; i < MACRO_COUNTOF(usr_malloc_str); i++)
	{
		if (usr_malloc_str[i].adr == adr)
		{
			usr_malloc_str[i].adr = NULL;
			usr_malloc_str[i].size = 0;
			free(adr);
			return;
		}
	}

	// Free failed
	printf("free is NG\n");
}

void usrMallcReport(void)
{
	int i;
	int use_cnt = 0;
	
	// Print malloc report info
	printf("--- usr malloc report ---\n");

	for (i = 0; i < MACRO_COUNTOF(usr_malloc_str); i++)
	{
		if (usr_malloc_str[i].adr != NULL)
		{
			use_cnt++;
			printf(" use  adr[%d] size[%d]\n", usr_malloc_str[i].adr, usr_malloc_str[i].size);
		}
	}

	printf("--- use cnt[%d] ---\n", use_cnt);
}
