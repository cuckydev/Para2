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
struct SysPad
{
	unsigned char data[32];
	unsigned int id;
	unsigned int status;
	unsigned char act_align[6];
	short unk6;
	char unk7[6];
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
	for (i = 0; i < 2; i++)
	{
		// Get pad state
		int state = scePadGetState(i, 0);
		WorkClear(sysPad[i].data, sizeof(sysPad[i].data));
		
		switch (state)
		{
			case scePadStateDiscon:
				// Pad not connected
				WorkClear(&sysPad[i], sizeof(sysPad[i]));
				break;
			case scePadStateFindCTP1:
			case scePadStateStable:
				switch (sysPad[i].status)
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
							sysPad[i].id = id;
							switch (id)
							{
								case 4:
									sysPad[i].status = 40;
									break;
								case 7:
									sysPad[i].status = 70;
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
							sysPad[i].status = 99;
							break;
						}
						sysPad[i].status++;
					}
				// Fallthrough
					case 41:
					{
						// Switch to analog mode
						if (scePadSetMainMode(i, 0, 1, 0) == 1)
							sysPad[i].status++;
						break;
					}
					case 42:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sysPad[i].status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sysPad[i].status = 0;
						break;
					}

					case 70:
					{
						// Check for Dualshock
						if (scePadInfoMode(i, 0, InfoModeIdTable, -1) == 0)
						{
							sysPad[i].status = 99;
							break;
						}
						if (scePadSetMainMode(i, 0, 1, 3) == 1)
							sysPad[i].status++;
						break;
					}
					case 71:
					case 77:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sysPad[i].status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sysPad[i].status = 80;
						break;
					}
					case 72:
					{
						if (scePadInfoPressMode(i, 0) == 1)
						{
							sysPad[i].status = 80;
							break;
						}
						sysPad[i].status = 76;
						break;
					}
					case 76:
					{
						if (scePadEnterPressMode(i, 0) == 1)
							sysPad[i].status++;
						break;
					}

					case 80:
					{
						// Get actuator information
						if (scePadInfoAct(i, 0, -1, 0) == 0)
							sysPad[i].status = 99;
						
						// Set actuator information
						sysPad[i].act_align[1] = 1;
						sysPad[i].act_align[5] = 0xFF;
						sysPad[i].act_align[0] = 0;
						sysPad[i].act_align[2] = 0xFF;
						sysPad[i].act_align[3] = 0xFF;
						sysPad[i].act_align[4] = 0xFF;
						if (scePadSetActAlign(i, 0, sysPad[i].act_align) == 0)
							break;
						
						sysPad[i].status++;
						break;
					}
					case 81:
					{
						// Check status
						if (scePadGetReqState(i, 0) == scePadReqStateFaild)
							sysPad[i].status--;
						if (scePadGetReqState(i, 0) == scePadReqStateComplete)
							sysPad[i].status = 99;
						break;
					}
				}
				break;
			default:
				break;
		}
	}
}

void padMakeData(Pad *pad, ushort x)
{
	
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
