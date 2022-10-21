#ifndef _ROM_UTIL_H
#define _ROM_UTIL_H

#include "memory.h"

void romInit();
void romCopy(const char *src, const char *dest, const int len);

#define LOAD_SEGMENT(segmentName, dest)                                 \
    dest = malloc((u32)(_ ## segmentName ## SegmentRomEnd - _ ## segmentName ## SegmentRomStart));                                     \
    romCopy(_ ## segmentName ## SegmentRomStart, dest, (u32)(_ ## segmentName ## SegmentRomEnd - _ ## segmentName ## SegmentRomStart));

#define CALC_ROM_POINTER(segmentName, addr) ((void*)(((unsigned)addr) - (unsigned)_ ## segmentName ## SegmentStart + (unsigned)_ ## segmentName ## SegmentRomStart))
#define CALC_RAM_POINTER(addr, ramStart)    (void*)(((unsigned)(addr) & 0xFFFFFF) + (unsigned)(ramStart))

#define ADJUST_POINTER_POS(ptr, offset) (void*)((ptr) ? (char*)(ptr) + (offset) : 0)

#define ADJUST_POINTER_FOR_SEGMENT(ptr, memorystart, segmentNumber) ((char*)ptr + (int)(memorystart) - ((segmentNumber) << 24))

#endif