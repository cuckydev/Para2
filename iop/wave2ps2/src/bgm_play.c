#include "bgm_i.h"

// Bgm play globals
static int oldstat;

static int bgmPlayReadMode;

static struct
{
	int size;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
	int unk5;
	int unk6;
	int unk7;
	int unk8;
	int unk9;
	int unkA;
	int unkB;
} wavep2;

int bug_bug_bug_flag = 0; // Someone wasn't happy

static sceCdlFILE fpCd;
int fp_pc = -1;

static sceCdRMode modeCd;

struct
{
	void *mem;
	void *ptr;
	int unk0;
	int db;
} sbuf = {};

int gThid = 0, gSem = 0;
int gThid_Tr = 0, gSem_Tr = 0;

int gBgmIntr = 0, gBgmIntrTime = 0;

void *ReadBuff = NULL;
static int ReadBuffSize;

int ReadOutCnt = 0;
int ReadOddEven = 0;

int BgmVolumeSet = 0;
int BgmPause = 0;
int BgmMode = 0;

// Thread functions
int IntFunc(int ch, void *common)
{
	if (gBgmIntr == 0)
	{
		iSignalSema(gSem);
		gBgmIntr = 1;
		ReadOddEven++;
	}
	else
	{
		sbuf.db ^= 1;
		bug_bug_bug_flag++;
	}

	gBgmIntrTime = 1;
	return 1;
}

void _BgmPlay(void);
int makeMyThread(void)
{
	struct ThreadParam param;
	param.attr = TH_C;
	param.entry = _BgmPlay;
	param.initPriority = 0x58;
	param.stackSize = 0x800;
	param.option = 0;
	return CreateThread(&param);
}

void _PreLoadBack(void);
int makeMyThread_Tr(void)
{
	struct ThreadParam param;
	param.attr = TH_C;
	param.entry = _BgmPlay;
	param.initPriority = 0x5A;
	param.stackSize = 0x800;
	param.option = 0;
	return CreateThread(&param);
}

int makeMySem(void)
{
	struct SemaParam param;
	param.initCount = 0;
	param.maxCount = 1;
	param.attr = 0;
	return CreateSema(&param);
}

// Bgm play functions
int BgmInit(int size)
{
	int i;

	// Initialize wavep2
	for (i = 0; i < sizeof(wavep2); i++)
		((char*)&wavep2)[0] = 0;
	
	// Create semaphores and threads
	if (gSem == 0)
		gSem = makeMySem();
	if (gThid == 0)
	{
		gThid = makeMyThread();
		StartThread(gThid, 0);
	}

	if (gSem_Tr == 0)
		gSem_Tr = makeMySem();
	if (gThid_Tr == 0)
	{
		gThid_Tr = makeMyThread_Tr();
		StartThread(gThid_Tr, 0);
	}

	// Allocate sbuf
	if (sbuf.mem == NULL)
	{
		CpuSuspendIntr(&oldstat);
		sbuf.mem = AllocSysMemory(0, 0xB000, NULL);
		CpuResumeIntr(oldstat);

		sbuf.ptr = sbuf.mem + 0x5800;
		sbuf.unk0 = 44;
	}
	sbuf.db = 0;

	// Allocate ReadBuff
	if (ReadBuff != NULL)
	{
		CpuSuspendIntr(&oldstat);
		FreeSysMemory(ReadBuff);
		CpuResumeIntr(oldstat);
	}

	CpuSuspendIntr(&oldstat);
	ReadBuff = AllocSysMemory(0, size << 9, NULL);
	CpuResumeIntr(oldstat);

	ReadBuffSize = size << 9;
	ReadOutCnt = 0;

	return gThid;
}

void BgmQuit(void)
{
	// Free ReadBuff and sbuf
	CpuSuspendIntr(&oldstat);

	FreeSysMemory(ReadBuff);
	ReadBuff = NULL;
	ReadBuffSize = 0;

	FreeSysMemory(sbuf.mem);
	sbuf.mem = NULL;
	sbuf.ptr = NULL;
	sbuf.unk0 = 0;

	CpuResumeIntr(oldstat);

	// Delete threads
	if (gThid != 0)
		TerminateThread(gThid);
	if (gSem != 0)
		DeleteSema(gSem);
	if (gThid != 0)
		DeleteThread(gThid);
	gThid = 0;
	gSem = 0;

	if (gThid_Tr != 0)
		TerminateThread(gThid_Tr);
	if (gSem_Tr != 0)
		DeleteSema(gSem_Tr);
	if (gThid_Tr != 0)
		DeleteThread(gThid_Tr);
	gThid_Tr = 0;
	gSem_Tr = 0;
}

int BgmOpen(const char *data)
{
	int open_result;

	// Check if we're requesting a PC file
	if (data[0] == 'h' && data[1] == 'o' && data[2] == 's' && data[3] == 't')
		bgmPlayReadMode = 1;
	else
		bgmPlayReadMode = 0;
	
	// Open file
	if (bgmPlayReadMode == 0)
	{
		// Find file on CD
		open_result = sceCdSearchFile(&fpCd, data);
	}
	else
	{
		// Find file on PC
		if (fp_pc >= 0)
			close(fp_pc);
		
		fp_pc = open(data, O_RDONLY);
		if (fp_pc >= 0)
		{
			fpCd.size = lseek(fp_pc, 0, SEEK_END);
			fpCd.lsn = 0;
			lseek(fp_pc, 0, SEEK_SET);
		}
		open_result = (fp_pc >= 0);
	}
	if (open_result == 0)
		return -1;
	
	// Init CD
	sceCdDiskReady(0);

	modeCd.trycount = 0;
	modeCd.spindlctrl = 0;
	modeCd.datapattern = 0;

	// Init bgm state
	ReadOutCnt = 0;
	gBgmIntr = 0;
	wavep2.size = fpCd.size >> 11;

	if (bgmPlayReadMode == 0)
		do {} while (sceCdStStart(fpCd.lsn, &modeCd));
	
	BgmMode |= 0x800;
	ReadOddEven = 0;

	CpuSuspendIntr(&oldstat);
	sceSdSetTransIntrHandler(1, IntFunc, 0);
	CpuResumeIntr(oldstat);

	return fpCd.size;
}

void _BgmPlay(void)
{

}

void _PreLoadBack(void)
{
	
}
