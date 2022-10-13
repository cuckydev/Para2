#include "os/mtc.h"

#include <eekernel.h>

#include "macro.h"

// Mtc constants
#define MTC_STACK_CHECK 0x572a8b4c

// Mtc stacks
char mtcStack_CTRL[0x1000] MACRO_AL16;
char mtcStack_MAIN[0x1000] MACRO_AL16;
char mtcStack_02[0x100] MACRO_AL16;
char mtcStack_03[0x800] MACRO_AL16;
char mtcStack_04[0x1000] MACRO_AL16;
char mtcStack_05[0x4000] MACRO_AL16;
char mtcStack_06[0x100] MACRO_AL16;
char mtcStack_07[0x4000] MACRO_AL16;
char mtcStack_08[0x100] MACRO_AL16;
char mtcStack_09[0x100] MACRO_AL16;
char mtcStack_0A[0x1000] MACRO_AL16;
char mtcStack_0B[0x100] MACRO_AL16;
char mtcStack_0C[0x100] MACRO_AL16;
char mtcStack_0D[0x1000] MACRO_AL16;
char mtcStack_0E[0x100] MACRO_AL16;
char mtcStack_0F[0x1000] MACRO_AL16;

// Mtc tasks
#define MTC_TASK_PRI 0x11

struct MtcTask
{
	short thread;
	int condition;
	unsigned char unk_exec;
	int unk12;
	struct ThreadParam param;
};

struct MtcTask mtcTaskConB[16];
int mtcCurrentTask;

// Mtc stack info
size_t mtcStackSizeTbl[32] = {
	sizeof(mtcStack_CTRL),
	sizeof(mtcStack_MAIN),
	sizeof(mtcStack_02),
	sizeof(mtcStack_03),
	sizeof(mtcStack_04),
	sizeof(mtcStack_05),
	sizeof(mtcStack_06),
	sizeof(mtcStack_07),
	sizeof(mtcStack_08),
	sizeof(mtcStack_09),
	sizeof(mtcStack_0A),
	sizeof(mtcStack_0B),
	sizeof(mtcStack_0C),
	sizeof(mtcStack_0D),
	sizeof(mtcStack_0E),
	sizeof(mtcStack_0F),
	0x1000, 0x1000, 0x100, 0x800,
	0x1000, 0x4000, 0x100, 0x4000,
	0x100, 0x100, 0x1000, 0x100,
	0x100, 0x1000, 0x100, 0x1000
};

char *mtcStack[16] = {
	mtcStack_CTRL,
	mtcStack_MAIN,
	mtcStack_02,
	mtcStack_03,
	mtcStack_04,
	mtcStack_05,
	mtcStack_06,
	mtcStack_07,
	mtcStack_08,
	mtcStack_09,
	mtcStack_0A,
	mtcStack_0B,
	mtcStack_0C,
	mtcStack_0D,
	mtcStack_0E,
	mtcStack_0F
};

// Mtc semaphore
struct SemaParam mtcSemaPara;

int mtcSemaEnd;

// Mtc control thread
#define MTC_CTRL_PRI 0x12

static struct ThreadParam th_para_Ctrl;
char mtcStack_Ctrl[0x1000] MACRO_AL16;

static short th_id_Ctrl;

void MtcChangeThCtrl(void *user)
{
	return;
}

// Mtc functions
static void mtcStackErrorCheck(int i)
{
	// Check stack cookie
	if (*((int*)&mtcStack[i]) != MTC_STACK_CHECK)
	{
		printf("stack over level[%d]\n", i);
		while (1) sceGsSyncV(0);
	}
}

void MtcInit(void)
{
	int i;

	// Clear tasks
	MACRO_ZERO(mtcTaskConB);

	// Create semaphore
	mtcSemaPara.maxCount = 1;
	mtcSemaPara.initCount = 0;

	mtcSemaEnd = CreateSema(&mtcSemaPara);

	// Create thread
	th_para_Ctrl.entry = MtcChangeThCtrl;
	th_para_Ctrl.stack = mtcStack_Ctrl;
	th_para_Ctrl.stackSize = sizeof(mtcStack_Ctrl);
	th_para_Ctrl.initPriority = MTC_CTRL_PRI;
	th_para_Ctrl.gpReg = &_gp;

	th_id_Ctrl = CreateThread(&th_para_Ctrl);
}

void MtcQuit(void)
{
	int i;

	// Delete thread and semaphore
	TerminateThread(th_id_Ctrl);
	DeleteThread(th_id_Ctrl);
	DeleteSema(mtcSemaEnd);

	// Kill tasks
	for (i = 0; i < MACRO_COUNTOF(mtcTaskConB); i++)
		MtcKill(i);
}

void MtcStart(void (*entry)(void*))
{
	// Execute task 0
	MtcExec(entry, 0);

	// Start thread
	mtcCurrentTask = -1;
	StartThread(th_id_Ctrl, 0);
	WaitSema(mtcSemaEnd);
}

void MtcExec(void (*entry)(void*), int i)
{
	// Kill previous task
	if (mtcTaskConB[i].condition != 0)
		MtcKill(i);
	
	// Create task thread
	mtcTaskConB[i].param.entry = entry;
	mtcTaskConB[i].param.option = i;
	mtcTaskConB[i].param.stack = mtcStack[i];
	mtcTaskConB[i].param.stackSize = mtcStackSizeTbl[i];
	mtcTaskConB[i].param.initPriority = MTC_TASK_PRI;
	mtcTaskConB[i].param.gpReg = &_gp;

	int thread = CreateThread(&mtcTaskConB[i].param);

	mtcTaskConB[i].condition = 2;
	mtcTaskConB[i].thread = thread;
	mtcTaskConB[i].unk_exec = 0;

	*((int*)&mtcStack[i]) = MTC_STACK_CHECK; // Set stack cookie
}

void MtcWait(unsigned char unk_exec)
{
	// Check stack
	FlushCache(0);
	mtcStackErrorCheck(mtcCurrentTask);

	// Set thread state
	mtcTaskConB[mtcCurrentTask].unk_exec = unk_exec;
	mtcTaskConB[mtcCurrentTask].condition = 0;

	// Wake up control thread and put this thread to sleep
	ChangeThreadPriority(th_id_Ctrl, MTC_CTRL_PRI);
	WakeupThread(th_id_Ctrl);

	SleepThread();
}

void MtcKill(int i)
{
	// Check status
	int condition = (mtcTaskConB[i].condition | ~0x8000); // BUG: Likely supposed to be AND
	if (condition != 0)
	{
		// Delete thread
		mtcTaskConB[i].condition = 0;
		if (condition != 2)
			TerminateThread(mtcTaskConB[i].thread);
		DeleteThread(mtcTaskConB[i].thread);
	}
}

void MtcPause(int i)
{
	// Set pause flag
	if (mtcTaskConB[i].condition != 0)
		mtcTaskConB[i].condition |= 0x8000;
}

void MtcContinue(int i)
{
	// Clear pause flag
	mtcTaskConB[i].condition &= ~0x8000;
}

void MtcExit(void)
{
	// Check stack
	mtcStackErrorCheck(mtcCurrentTask);

	// Set thread state
	mtcTaskConB[mtcCurrentTask].condition = 0;

	// Wake up control thread and delete this thread
	ChangeThreadPriority(th_id_Ctrl, MTC_CTRL_PRI);
	WakeupThread(th_id_Ctrl);

	ExitDeleteThread();
}

int MtcGetCondition(int i)
{
	return mtcTaskConB[i].condition;
}

int MtcResetCheck(void)
{
	return 0;
}
