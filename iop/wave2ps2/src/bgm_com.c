#include "bgm_i.h"
#include <bgmcmd.h>

extern void EnableIntr(int);

// RPC arguments
int gRpcArg[16];

// Bgm thread
static void *bgmFunc(unsigned int command, void *data, int size);

int sce_bgm_loop(void)
{
	sceSifQueueData rpc_qd;
	sceSifServeData rpc_sd;

	// Initialize interrupts
	CpuEnableIntr();
	EnableIntr(0x24);
	EnableIntr(0x28);
	EnableIntr(0x09);

	// Initialize RPC
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&rpc_qd, GetThreadId());
	sceSifRegisterRpc(&rpc_sd, sce_BGM_DEV, bgmFunc, (void*)gRpcArg, NULL, NULL, &rpc_qd);

	return 0;
}

// Bgm function
int ret = 0;
static void *bgmFunc(unsigned int command, void *data, int size)
{
	// ret = 0;
	
	switch (command)
	{
		default:
			printf("EzBGM driver error: unknown command %d \n", *((int*)data + 0));
			break;
	}

	return (void*)&ret;
}
