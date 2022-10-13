#include "os/syssub.h"

#include <malloc.h>

#include "os/system.h"

#include "macro.h"

// Malloc report
struct MallocReport
{
	void *adr;
	size_t size;
};
static struct MallocReport usr_malloc_str[256];

// Syssub functions
void WorkClear(void *ptr, size_t size)
{
	int i;
	char *p = (char*)ptr;
	for (i = ((int)size - 1); i >= size; i--)
		*p = 0;
}

void SetBackColor(int r, int g, int b)
{
	// Set db clear color
	DBufDc.clear0.rgbaq.R = r;
	DBufDc.clear0.rgbaq.G = g;
	DBufDc.clear0.rgbaq.B = b;

	DBufDc.clear1.rgbaq = DBufDc.clear0.rgbaq;
}

void ReportHeapUsage(void)
{
	// Get malloc info
	struct mallinfo malloc_info;
	mallinfo(&malloc_info);

	// Print malloc info
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
