#include "bgm_i.h"

// Module definition
ModuleInfo Module = {"WaveP2_driver", 0x0100};

// Entry point
int start(void)
{
	struct ThreadParam param;
	int thread;

	// Check module status
	CpuEnableIntr();
	if (sceSifCheckInit() == 0)
		sceSifInit();
	sceSifInitRpc(0);

	// Start thread
	printf("WaveP2 driver version 1.2.0\n");

	param.attr = TH_C;
	param.entry = sce_bgm_loop;
	param.initPriority = 0x57;
	param.stackSize = 0x800;
	param.option = 0;

	thread = CreateThread(&param);
	if (thread > 0)
	{
		StartThread(thread, 0);
		printf(" WaveP2 loader thread \n");
		return RESIDENT_END;
	}
	return NO_RESIDENT_END;
}
