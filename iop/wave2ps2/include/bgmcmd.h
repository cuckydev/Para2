#ifndef BGM_H
#define BGM_H

// Bgm module ID
#define sce_BGM_DEV 0x8800

// Bgm commands
#define rBgmInit            0x8000
#define rBgmQuit            0x0001
#define rBgmOpen            0x8002
#define rBgmClose           0x0003
#define rBgmPreLoad         0x0004
#define rBgmStart           0x8005
#define rBgmStop            0x0006
#define rBgmSeek            0x8007
#define rBgmSetVolume       0x0008
#define rBgmSetVolumeDirect 0x0009
#define rBgmSetMasterVolume 0x000A
#define rBgmSetMode         0x800C
#define rBgmGetMode         0x800B
#define rBgmSdInit          0x000D
#define rBgmSetChannel      0x000F
#define rBgmCdInit          0x000E
#define rBgmGetTime         0x8010
#define rBgmGetTSample      0x8011
#define rBgmGetCdErrCode    0x8012
#define rBgmOpenFLoc        0x8013
#define rBgmSeekFLoc        0x8014
#define rBgmPreLoadBack     0x0015
#define rBgmSetTrPoint      0x0016
#define rBgmReadBuffFull    0x8017

#endif
