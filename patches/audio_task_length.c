#include "patches.h"
#include "game/audio/audio_engine_internal.h"
#include "audio_task_length.h"

RECOMP_PATCH s32 buildAudioTask(AudioTask *task, AudioInfo *info) {
    u32 outBuf;
    AudioTask *task3;
    s32 cmdLen[3];
    AudioTask *task2;
    Acmd *cmdListEnd;

    reclaimAudioDmaBuffers();
    outBuf = osVirtualToPhysical(task->outBuf);

    if (info != NULL) {
        if (!aspMainTextStart) {}
        osAiSetNextBuffer(info->buf, info->len * 4);
    }

    // @recomp Clamp host queue-derived lengths before narrowing to s16.
    task->outLen = audioTaskOutputLength(osAiGetLength(), gTargetAudioTaskOutputLen,
                                        gMinAudioTaskOutputLen, gMaxAudioTaskOutputLen);

    cmdListEnd =
        alAudioFrame(gAudioWorkBuffers.commandLists[gAudioCmdListIndex], &cmdLen[2], (s16 *)outBuf, task->outLen);
    if (cmdLen[2] == 0) {
        return 0;
    }

    task3 = task;
    task3->unk8 = 0;
    task3->msgQ = (OSMesgQueue *)gAudioTaskDoneQueue;
    task3->msg = (OSMesg)&task3->unk68;
    task3->unk10 = 0;
    task3->dataPtr = gAudioWorkBuffers.commandLists[gAudioCmdListIndex];
    task3->dataSize = (((s32)cmdListEnd - (s32)gAudioWorkBuffers.commandLists[gAudioCmdListIndex]) >> 3) << 3;

    task3->type = 2;
    task3->ucodeBoot = rspbootTextStart;
    task2 = task3;
    task2->ucodeBootSize = (u8 *)aspMainTextStart - (u8 *)rspbootTextStart;
    task3->flags = 0;
    task3->ucode = aspMainTextStart;
    task3->ucodeData = aspMainDataStart;
    task3->ucodeDataSize = 0x800;
    task3->dramStack = NULL;
    task3->dramStackSize = 0;
    task3->outputBuff = NULL;
    task3->outputBuffSize = NULL;
    task3->yieldDataPtr = NULL;
    task3->yieldDataSize = 0;

    osSendMesg(getSchedulerAudioTaskQueue(gAudioSchedulerState), &task3->unk8, 1);
    gAudioCmdListIndex ^= 1;
    return 1;
}
