// version 1.0

#pragma once

#include "include/BinaryReader.h"
#include "include/afctool.h"

#define STRM_HEADER_SIZE 0x40
#define BLCK_HEADER_SIZE 0x20
#define BLCK_BLOCK_SIZE 0x2760
// ADPCM_SAMPLES_PER_BLOCK = 17920
#define ADPCM_SAMPLES_PER_BLOCK (u32)((u32)BLCK_BLOCK_SIZE / (u32)BYTES_PER_FRAME * (u32)SAMPLES_PER_FRAME)

namespace AST {
    u32 getNumBlckHeaders(u32 numSamples);
    u32 getBufferSize(u32 numSamples, u32 channels);
    bool makeAst(s16**, BinaryReader& outFile, u32 numSamples, u32 channels, u32 samplerate, bool isLooped, u32 loopStart, u32 loopEnd);
    bool convertAstPcm16ToAstAdpcm(BinaryReader& inAst, BinaryReader& outAst);
}