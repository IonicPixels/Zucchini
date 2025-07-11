// version 1.0

#pragma once

#include "include/BinaryReader.h"

namespace WAV {
    class WAV_INFO : public BinaryReader {
    private:
        bool parseFmt();
        bool parseData();
        bool parseSmpl();

        u32 mWaveDataSize = 0;
        u32 mSampleRate = 0;
        u32 mChannelCount = 0;
        u32 mNumSamples = 0;
        u32 mDataChunkOfs = 0;
        bool mHasLoop = false;
        u32 mLoopStart = 0;
        u32 mLoopEnd = 0;

    public:
        WAV_INFO(const char* pFile) : BinaryReader(pFile) {}

        bool parseFile();
        s16** makeWaveDataBuffers();
        u32 getWaveDataSize() const;
        u32 getNumSamples() const;
        u32 getNumChannels() const;
        u32 getSamplerate() const;
        
        bool isLooped() const;
        u32 getLoopStartSample() const;
        u32 getLoopEndSample() const;
    };
}