#ifndef BGM_I_H
#define BGM_I_H

#include <kernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <introld.h>

#include <sys/types.h>
#include <stdio.h>

// Module ID number
#define sce_BGM_DEV 0x8800

// Bgm functions
int sce_bgm_loop(void);

#endif
