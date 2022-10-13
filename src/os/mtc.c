#include "os/mtc.h"

#include <eekernel.h>

// Mtc semaphore
struct SemaParam mtcSemaPara;

// Mtc thread
struct ThreadParam th_para_Ctrl;
char mtcStack_Ctrl[0x1000];

void MtcChangeThCtrl(void)
{
    return;
}

// Mtc functions
void MtcInit(void)
{
    // Create semaphore
    mtcSemaPara.maxCount = 1;
    mtcSemaPara.initCount = 0;
    int sema = CreateSema(&mtcSemaPara);

    // Create thread
    th_para_Ctrl.entry = MtcChangeThCtrl;
    th_para_Ctrl.stack = mtcStack_Ctrl;
    th_para_Ctrl.stackSize = sizeof(mtcStack_Ctrl);
    th_para_Ctrl.initPriority = 0x12;
    th_para_Ctrl.gpReg = _gp;
    int thread = CreateThread(&th_para_Ctrl);
}
