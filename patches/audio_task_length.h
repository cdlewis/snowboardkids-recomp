#ifndef AUDIO_TASK_LENGTH_H
#define AUDIO_TASK_LENGTH_H

static inline int audioTaskOutputLength(unsigned int queuedBytes, int target, int minimum, int maximum) {
    int frames = target - (int)(queuedBytes >> 2) + 0x68;
    if (frames < minimum) {
        frames = minimum;
    }
    if (frames > maximum) {
        frames = maximum;
    }
    return frames & ~0xF;
}

#endif
