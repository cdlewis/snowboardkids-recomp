#include "patches.h"

#define FRAMEBUFFER_COUNT 3
#define SCHEDULER_SWAPBUFFER_FLAG 0x40
#define SCHEDULER_RETRACE_MASK 0xFFF
#define FRAMEBUFFER_SWAP_RETRACE_DELAY 3
#define RSP_OUTPUT_BUFFER_SIZE 0x8000
#define RSP_UCODE_DATA_SIZE 0x800
#define RSP_DRAM_STACK_SIZE 0x400
#define RSP_YIELD_BUFFER_SIZE 0xC00

extern Gfx *gRegionAllocPtr;
extern u8 gFramebufferColorBufferIndex;
extern s32 gClearFramebufferOnNextTask;
extern u16 gLastSchedulerRetraceCounter;
extern OSMesgQueue gFramebufferRenderDoneQueue;
extern SchedulerState gSchedulerState;

extern u8 D_369000[];

extern void selectMenuRenderScratchBuffer(s32 frameIndex);
extern void appendViewportDisplayLists(u8 frameIndex);

RECOMP_PATCH void submitFramebufferRenderTask(u8 frameIndex) {
    SchedulerTask *schedulerTask;

    gFramebufferColorBufferIndex++;
    if (gFramebufferColorBufferIndex >= FRAMEBUFFER_COUNT) {
        gFramebufferColorBufferIndex = 0;
    }

    gFrameRenderTasks[frameIndex].framebuffer = gFramebuffers[gFramebufferColorBufferIndex];
    selectMenuRenderScratchBuffer(frameIndex);

    gRegionAllocPtr = gFrameRenderTasks[frameIndex].displayList;
    gCurrentFrameRenderData = &gFrameRenderTasks[frameIndex].renderData;
    schedulerTask = &gFrameRenderTasks[frameIndex].schedulerTask;

    // @recomp Enable RT64's extended GBI before emitting the HUD alignment commands.
    gEXEnable(gRegionAllocPtr++);
    gSPSegment(gRegionAllocPtr++, 0, 0);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, 320, 240);
    gSPClearGeometryMode(gRegionAllocPtr++,
                         G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_SHADING_SMOOTH);
    gSPClipRatio(gRegionAllocPtr++, FRUSTRATIO_1);
    gDPPipelineMode(gRegionAllocPtr++, G_PM_NPRIMITIVE);
    gDPSetTextureImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_369000);

    if (gClearFramebufferOnNextTask != 0) {
        gClearFramebufferOnNextTask = 0;
        gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetColorImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBuffer);
        gDPSetFillColor(gRegionAllocPtr++, 0xFFFCFFFC);
        gDPFillRectangle(gRegionAllocPtr++, 0, 0, 319, 239);
        gDPSetDepthImage(gRegionAllocPtr++, gDepthBuffer);
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetColorImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameRenderTasks[frameIndex].framebuffer);
        gDPSetFillColor(gRegionAllocPtr++, 0x10001);
        gDPFillRectangle(gRegionAllocPtr++, 0, 0, 319, 239);
        gDPSetEnvColor(gRegionAllocPtr++, 0, 0, 0, 0);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, 0);
        gDPSetBlendColor(gRegionAllocPtr++, 0, 0, 0, 0);
        gDPSetFogColor(gRegionAllocPtr++, 0, 0, 0, 0);
        gDPSetFillColor(gRegionAllocPtr++, 0);
        gDPSetPrimDepth(gRegionAllocPtr++, 0, 0);
        gDPSetConvert(gRegionAllocPtr++, 0, 0, 0, 0, 0, 0);
        gDPSetKeyR(gRegionAllocPtr++, 0, 0, 0);
        gDPSetKeyGB(gRegionAllocPtr++, 0, 0, 0, 0, 0, 0);
        gDPSetCombineKey(gRegionAllocPtr++, G_CK_NONE);
        gDPNoOp(gRegionAllocPtr++);
        gDPSetTileSize(gRegionAllocPtr++, 0, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 1, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 2, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 3, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 4, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 5, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 6, 0, 0, 0, 0);
        gDPSetTileSize(gRegionAllocPtr++, 7, 0, 0, 0, 0);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 0,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 1,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 2,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 3,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 4,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 5,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 6,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTile(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0, 7,
                   0, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
    } else {
        // @recomp Inline the clear that the deleted framebuffer-prepare task used to
        // perform, targeting this frame's buffers instead of the next frame's.
        gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetColorImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBuffer);
        gDPSetFillColor(gRegionAllocPtr++, 0xFFFCFFFC);
        gDPFillRectangle(gRegionAllocPtr++, 0, 0, 319, 239);
        gDPSetDepthImage(gRegionAllocPtr++, gDepthBuffer);
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetColorImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameRenderTasks[frameIndex].framebuffer);
        gDPSetFillColor(gRegionAllocPtr++, 0x10001);
        gDPFillRectangle(gRegionAllocPtr++, 0, 0, 319, 239);
    }

    gDPSetAlphaCompare(gRegionAllocPtr++, G_AC_NONE);
    appendViewportDisplayLists(frameIndex);
    gDPFullSync(gRegionAllocPtr++);
    gSPEndDisplayList(gRegionAllocPtr++);

    schedulerTask->rspTask.t.data_ptr =
        (u64 *)(gCurrentFrameRenderData + 1);
    schedulerTask->rspTask.t.data_size =
        (gRegionAllocPtr - (Gfx *)(gCurrentFrameRenderData + 1)) * sizeof(Gfx);
    schedulerTask->rspTask.t.type = M_GFXTASK;
    schedulerTask->rspTask.t.flags = 0;
    schedulerTask->rspTask.t.ucode_boot = (u64 *)rspbootTextStart;
    schedulerTask->rspTask.t.ucode_boot_size = (unsigned long)aspMainTextStart - (unsigned long)rspbootTextStart;
    schedulerTask->rspTask.t.ucode = (u64 *)gF3dlxMicrocodeText;
    schedulerTask->rspTask.t.ucode_data = (u64 *)gspF3DLX_fifoDataStart;
    schedulerTask->rspTask.t.ucode_data_size = RSP_UCODE_DATA_SIZE;
    schedulerTask->rspTask.t.dram_stack = (u64 *)gRspDramStack;
    schedulerTask->rspTask.t.dram_stack_size = RSP_DRAM_STACK_SIZE;
    schedulerTask->rspTask.t.output_buff = (u64 *)gRspOutputBuffer;
    schedulerTask->rspTask.t.output_buff_size =
        (u64 *)((unsigned long)gRspOutputBuffer + (long long)RSP_OUTPUT_BUFFER_SIZE);
    schedulerTask->rspTask.t.yield_data_ptr = (u64 *)gRspYieldBuffer;
    schedulerTask->rspTask.t.yield_data_size = RSP_YIELD_BUFFER_SIZE;
    schedulerTask->next = NULL;
    schedulerTask->flags = SCHEDULER_SWAPBUFFER_FLAG;
    schedulerTask->doneQueue = &gFramebufferRenderDoneQueue;
    schedulerTask->doneMsg = &gFrameRenderTasks[frameIndex].completionMessage;
    schedulerTask->framebuffer = gFrameRenderTasks[frameIndex].framebuffer;
    schedulerTask->retrace =
        SCHEDULER_RETRACE_MASK & (gLastSchedulerRetraceCounter + FRAMEBUFFER_SWAP_RETRACE_DELAY);
    gFrameRenderTasks[frameIndex].status |= 1;
    osSendMesg(getSchedulerGraphicsTaskQueue(&gSchedulerState), schedulerTask, 1);

    // @recomp The original function continued here by building a second display list. That
    // whole block is deleted. Its work now happens inline at the head of the list above.
}
