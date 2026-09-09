#include "../patches/audio_task_length.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const int32_t targets[] = {368, 544, 736, 800};
    unsigned checked = 0;
    for (unsigned t = 0; t < sizeof(targets) / sizeof(targets[0]); t++) {
        int32_t target = targets[t];
        int32_t minimum = target - 16;
        int32_t maximum = target + 104;
        for (uint32_t queued = 0; queued < 262144; queued++) {
            int32_t length = audioTaskOutputLength(queued, target, minimum, maximum);
            assert(length >= minimum && length <= maximum && length <= INT16_MAX);
            assert((length & 15) == 0);
            if ((queued >> 2) <= (uint32_t)(target + 104)) {
                int32_t original = (target - (int32_t)(queued >> 2) + 104) & 0xFFF0;
                if (original < minimum) original = minimum;
                assert(length == original);
            }
            checked++;
        }
        assert(audioTaskOutputLength(UINT32_MAX, target, minimum, maximum) == minimum);
        uint32_t crashBacklog = (uint32_t)(target + 104 + 688) * 4;
        int16_t original = (int16_t)((target - (crashBacklog >> 2) + 104) & 0xFFF0);
        assert(original == -688);
        assert((uint32_t)(original * 4) / 2 == 2147482272u);
        assert(audioTaskOutputLength(crashBacklog, target, minimum, maximum) == minimum);
    }
    printf("PASS: %u queue lengths, boundary cases, and crash-register reproduction\n", checked);
}
