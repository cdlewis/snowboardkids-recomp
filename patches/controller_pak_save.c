#include "patches.h"

#include "game/save_data.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"

typedef struct SavedNoteState {
    u32 file_size;
    u32 game_code;
    u16 company_code;
    char ext_name[4];
    char game_name[16];
} SavedNoteState;

typedef struct SaveHeader {
    u32 magic;
    u32 version;
    u32 payloadSize;
    u32 reserved;
} SaveHeader;

extern volatile SaveFileIdentity gControllerPakSaveFileIdentity;
extern SavedNoteState gControllerPakFileStates[];
extern u8 gControllerPakSaveExtNameBytesEnd[];

s32 recomp_save_read(u32 offset, void* dst, u32 size);
s32 recomp_save_write(u32 offset, const void* src, u32 size);

#define SAVE_MAGIC 0x534B3153
#define SAVE_VERSION 1
#define SAVE_HEADER_OFFSET 0
#define SAVE_HEADER_SIZE 0x10
#define SAVE_PAYLOAD_OFFSET SAVE_HEADER_SIZE
#define SAVE_PAYLOAD_SIZE CONTROLLER_PAK_SAVE_READ_SIZE
#define PAK_MAX_NOTES 16
#define PAK_TOTAL_PAGES 123
#define PAK_PAGE_SIZE 256
#define SAVE_GAME_CODE 0x4E534B45
#define SAVE_COMPANY_CODE 0x4542

typedef char SavedNoteStateSizeCheck[(sizeof(SavedNoteState) == 0x20) ? 1 : -1];
typedef char SaveHeaderSizeCheck[(sizeof(SaveHeader) == SAVE_HEADER_SIZE) ? 1 : -1];

static s32 savedGameExists(void) {
    SaveHeader header;

    if (!recomp_save_read(SAVE_HEADER_OFFSET, &header, SAVE_HEADER_SIZE)) {
        return 0;
    }

    return (header.magic == SAVE_MAGIC) && (header.version == SAVE_VERSION) &&
           (header.payloadSize == SAVE_PAYLOAD_SIZE);
}

static s32 writeSavedGame(GameSaveData* save) {
    SaveHeader header;

    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.payloadSize = SAVE_PAYLOAD_SIZE;
    header.reserved = 0;

    return recomp_save_write(SAVE_PAYLOAD_OFFSET, save, SAVE_PAYLOAD_SIZE) &&
           recomp_save_write(SAVE_HEADER_OFFSET, &header, SAVE_HEADER_SIZE);
}

static s32 deleteSavedGame(u16 fileIndex, u16 companyCode, u32 gameCode, u8 gameName[16], u8 extName[4]) {
    SaveHeader header;

    (void) companyCode;
    (void) gameCode;
    (void) gameName;
    (void) extName;

    if ((fileIndex != 0) || !savedGameExists()) {
        return 1;
    }

    header.magic = 0;
    header.version = 0;
    header.payloadSize = 0;
    header.reserved = 0;
    return recomp_save_write(SAVE_HEADER_OFFSET, &header, SAVE_HEADER_SIZE);
}

static void readSavedGameFileState(s32 fileIndex, SavedNoteState* state) {
    s32 i;

    state->file_size = 0;
    state->game_code = 0;
    state->company_code = 0;
    for (i = 0; i < 4; i++) {
        state->ext_name[i] = 0;
    }
    for (i = 0; i < 16; i++) {
        state->game_name[i] = 0;
    }

    if ((fileIndex == 0) && savedGameExists()) {
        state->file_size = CONTROLLER_PAK_SAVE_NOTE_SIZE;
        state->game_code = SAVE_GAME_CODE;
        state->company_code = SAVE_COMPANY_CODE;
        for (i = 0; i < 4; i++) {
            state->ext_name[i] = gControllerPakSaveExtNameBytes[i];
        }
        for (i = 0; i < 16; i++) {
            state->game_name[i] = gControllerPakSaveGameNameBytes[i];
        }
    }
}

RECOMP_PATCH void probeControllerPak(u16 arg0) {
    u32 ret;

    // @recomp Treat the recomp-managed save as an available Controller Pak.
    ret = 0;
    if (ret == 2) {
        ret = 0;
    }

    if (ret == 0) {
        gControllerPakStatusCodes[arg0] = 1;
    }

    if ((ret == 1) || (ret == 11)) {
        gControllerPakStatusCodes[arg0] = 10;
    }

    if (ret == 10) {
        if (gRumblePakConnectedByController[arg0] == 1) {
            gControllerPakStatusCodes[arg0] = 16;
        } else {
            gControllerPakStatusCodes[arg0] = 7;
        }
    }

    if (ret != 0) {
        gControllerPakOperationCounts[arg0]++;
    }
}

RECOMP_PATCH void checkControllerPakSaveStatus(u16 arg0) {
    s32 ret;
    s32 maxFiles;
    s32 filesUsed;
    s32 freeBytes;
    s32 i;

    gControllerPakSaveFileIdentity.size = CONTROLLER_PAK_SAVE_NOTE_SIZE;
    gControllerPakSaveFileIdentity.gameCode = 'NSKE';
    gControllerPakSaveFileIdentity.companyCode = 'EB';

    i = 0;
    do {
        gControllerPakSaveFileIdentity.extName[i] = gControllerPakSaveExtNameBytes[i];
        i++;
    } while (i < 4);

    i = 0;
    do {
        gControllerPakSaveFileIdentity.gameName[i] = gControllerPakSaveGameNameBytes[i];
        i++;
    } while (i < 16);

    // @recomp Find the save in the recomp-managed buffer instead of through libultra's Pak API.
    ret = savedGameExists() ? 0 : 5;
    if (ret == 0) {
        gControllerPakStatusCodes[arg0] = 2;
    } else {
        // @recomp Report the virtual Pak's file usage.
        maxFiles = PAK_MAX_NOTES;
        filesUsed = savedGameExists() ? 1 : 0;
        if (filesUsed == 0x10) {
            gControllerPakStatusCodes[arg0] = 0xC;
        } else {
            // @recomp Report the virtual Pak's remaining space.
            freeBytes = (PAK_TOTAL_PAGES * PAK_PAGE_SIZE) - (filesUsed * CONTROLLER_PAK_SAVE_NOTE_SIZE);
            maxFiles = freeBytes / 256;
            if (maxFiles < 0x79) {
                gControllerPakStatusCodes[arg0] = 0xB;
            } else if (ret == 5) {
                gControllerPakStatusCodes[arg0] = 9;
            }
        }
    }

    if (ret != 0) {
        gControllerPakOperationCounts[arg0]++;
    }
}

RECOMP_PATCH void readControllerPakSave(u16 controllerIndex) {
    u8 *cursor;
    s32 offset;
    s32 readStatus;
    u16 checksumFailed;
    GameSaveData *save;
    s32 checksum;

    checksumFailed = 0;

    gControllerPakSaveFileIdentity.gameCode = 'NSKE';
    gControllerPakSaveFileIdentity.companyCode = 'EB';

    offset = 0;
    do {
        gControllerPakSaveFileIdentity.extName[offset] = gControllerPakSaveExtNameBytes[offset];
        offset++;
    } while (&gControllerPakSaveExtNameBytes[offset] < gControllerPakSaveExtNameBytesEnd);

    offset = 0;
    do {
        gControllerPakSaveFileIdentity.gameName[offset] = gControllerPakSaveGameNameBytes[offset];
        offset++;
    } while (&gControllerPakSaveGameNameBytes[offset] < gControllerPakSaveExtNameBytes);

    save = &gGameSaveDataBuffer[controllerIndex];
    // @recomp Read the shared save from the recomp-managed buffer instead of a Controller Pak file.
    readStatus = !savedGameExists() || !recomp_save_read(SAVE_PAYLOAD_OFFSET, save, SAVE_PAYLOAD_SIZE);

    if (readStatus == 0) {
        checksum = 0;
        cursor = (u8 *)save->highScores;
        offset = CONTROLLER_PAK_CHECKSUM_START_OFFSET;
        do {
            checksum += *cursor++;
            offset++;
        } while (offset != CONTROLLER_PAK_SAVE_READ_SIZE);

        if (checksum != save->checksum) {
            checksumFailed = 1;
        }

        cursor = &gControllerPakRetryCounts[controllerIndex];
        if (checksumFailed == 0) {
            if (validateControllerPakSave(controllerIndex) == 0) {
                gControllerPakRetryCounts[controllerIndex] = 0;
            }
            cursor = &gControllerPakRetryCounts[controllerIndex];
        } else {
            (*cursor)++;
        }
    } else {
        goto read_failed;
    read_failed:
        cursor = &gControllerPakRetryCounts[controllerIndex];
        (*cursor)++;
    }

    if ((readStatus != 0) || (*cursor != 0)) {
        if (*cursor != CONTROLLER_PAK_MAX_READ_RETRIES) {
            return;
        }
    }
    gControllerPakOperationCounts[controllerIndex]++;
}

RECOMP_PATCH void writeControllerPakSave(u16 controllerIndex) {
    GameSaveData *save;
    u8 *src;
    s32 i;
    s32 checksum;

    gControllerPakSaveFileIdentity.size = CONTROLLER_PAK_SAVE_NOTE_SIZE;
    gControllerPakSaveFileIdentity.gameCode = 'NSKE';
    gControllerPakSaveFileIdentity.companyCode = 'EB';

    i = 0;
    do {
        gControllerPakSaveFileIdentity.extName[i] = gControllerPakSaveExtNameBytes[i];
        i++;
    } while (&gControllerPakSaveExtNameBytes[i] < gControllerPakSaveExtNameBytesEnd);

    i = 0;
    do {
        gControllerPakSaveFileIdentity.gameName[i] = gControllerPakSaveGameNameBytes[i];
        i++;
    } while (&gControllerPakSaveGameNameBytes[i] < gControllerPakSaveExtNameBytes);

    save = &gGameSaveDataBuffer[controllerIndex];
    checksum = 0;
    src = (u8 *)save->highScores;
    i = 4;
    do {
        checksum += *src++;
        i++;
    } while (i != 0x78E0);
    gGameSaveDataBuffer[controllerIndex].checksum = checksum;

    // @recomp Write the shared save through the recomp framework instead of a Controller Pak file.
    if (writeSavedGame(save)) {
        gControllerPakRetryCounts[controllerIndex] = 0;
    } else {
        gControllerPakRetryCounts[controllerIndex]++;
    }
}

RECOMP_PATCH void repairControllerPakId(u16 arg0) {
    s32 ret;

    // @recomp The recomp-managed save has no Controller Pak ID to repair.
    ret = 0;
    if ((ret == 4) || (ret == 0xA)) {
        gControllerPakRetryCounts[arg0] += 1;
    }
}

RECOMP_PATCH void readControllerPakFileStates(void) {
    s32 i;

    for (i = 0; i != 0x10; i++) {
        // @recomp Populate each file slot from the recomp-managed save.
        readSavedGameFileState(i, &gControllerPakFileStates[i]);
    }
}

RECOMP_PATCH void deleteControllerPakFile(u16 arg0) {
    SavedNoteState *state;
    u16 companyCode;
    u32 gameCode;
    u8 gameName[16];
    u8 extName[4];
    s32 i;

    state = &gControllerPakFileStates[arg0];
    companyCode = state->company_code;
    gameCode = state->game_code;

    for (i = 0; i < 16; i++) {
        gameName[i] = gControllerPakFileStates[arg0].game_name[i];
    }

    for (i = 0; i < 4; i++) {
        extName[i] = gControllerPakFileStates[arg0].ext_name[i];
    }

    for (i = 0; i != 3; i++) {
        // @recomp Delete the selected virtual file from the recomp-managed save.
        if (deleteSavedGame(arg0, companyCode, gameCode, gameName, extName)) {
            gControllerPakRetryCounts[0] = 0;
            return;
        }
        gControllerPakRetryCounts[0]++;
    }
}

RECOMP_PATCH void updateControllerPakFreeSpaceInfo(void) {
    s32 pad;
    s32 maxFiles;
    s32 filesUsed;

    // @recomp Calculate free space from the single virtual save file.
    filesUsed = savedGameExists() ? 1 : 0;
    maxFiles = PAK_MAX_NOTES;
    gControllerPakFreeBytes = (PAK_TOTAL_PAGES * PAK_PAGE_SIZE) - (filesUsed * CONTROLLER_PAK_SAVE_NOTE_SIZE);
    gControllerPakFreeFileCount = maxFiles - filesUsed;
}
