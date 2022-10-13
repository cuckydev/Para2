#ifndef OS_MTC_H
#define OS_MTC_H

// Mtc functions
void MtcInit(void);
void MtcQuit(void);
void MtcStart(void (*entry)(void*));
void MtcExec(void (*entry)(void*), int i);
void MtcWait(unsigned char unk_exec);
void MtcKill(int i);
void MtcPause(int i);
void MtcContinue(int i);
void MtcExit(void);
int MtcGetCondition(int i);
int MtcResetCheck(void);

#endif
