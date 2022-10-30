
#include "time.h"

float gTimePassed = 0.0f;

void timeUpdateDelta() {
    gTimePassed += FIXED_DELTA_TIME;

}