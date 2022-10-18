#ifndef BGM_PLAY_H
#define BGM_PLAY_H

#include "bgm_i.h"

// Bgm play functions
int BgmInit(int size);
void BgmQuit(void);
int BgmOpen(const char *name);

#endif
