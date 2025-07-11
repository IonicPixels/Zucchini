// version 1.0

#pragma once

#include "include/Types.h"
#include "include/BinaryReader.h"

#define BYTES_PER_FRAME 9
#define SAMPLES_PER_FRAME 16

namespace AFC {
    static const s16 sAdpcmCoefficents[16][2] = {
        {    0,     0 }, {  2048,     0 }, {     0,  2048 }, {  1024,  1024 },
        { 4096, -2048 }, {  3584, -1536 }, {  3072, -1024 }, {  4608, -2560 },
        { 4200, -2248 }, {  4800, -2300 }, {  5120, -3072 }, {  2048, -2048 },
        { 1024, -1024 }, { -1024,  1024 }, { -1024,     0 }, { -2048,     0 }
    };

    static const s16 sAdpcmNibbleToInt[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1
    };

    class ADPCM_INFO {
    private:
        void decodeFrame(const u8 adpcm[BYTES_PER_FRAME], s16 dst[SAMPLES_PER_FRAME]);
        void encodeFrame(u8 dst[BYTES_PER_FRAME], u32 currSamp);

        u32 mNumSamples = 0;
        s16* mPCM = 0; // source
        u8* mADPCM = 0; // source
        s16 mLast = 0;
        s16 mPenult = 0;
        
        // loop related members
        bool mIsLooped = false;
        bool mIsRequire0CoefAtLoop = false;
        u32 mSourceNumSamples = 0;
        u32 mSourceLoopStart = 0;
        u32 mLoopStart = 0;
        u32 mLoopEnd = 0;
        s16 mLoopLast = 0;
        s16 mLoopPenult = 0;

    public:
        ADPCM_INFO() {}

        bool setInputPCMData(const BinaryReader&, u32 dataLen); // call this function first!
        void setInputPCMData(s16*, u32 numSamples); // call this function first!
        bool setInputADPCMData(const BinaryReader&, u32 dataLen); // call this function first!
        void setLoopData(u32 loopStart, u32 loopEnd, bool requireCoef0AtLoopStart); // requireCoef0AtLoopStart only required to be set to "true" for AST

        u8* createADPCMBuffer() const;
        u32 getADPCMBufferSize() const;

        s16* createPCMBuffer() const;
        u32 getPCMBufferSize() const;

        void encode(u8* dst);
        void encode(u8* dst, u32 startSample, u32 numSamples);
        void printEncodeInfoStart() const;
        void printEncodeInfoEnd() const;
        void decode(s16* dst);
        s16 getLast() const;
        s16 getPenult() const;
        s16 getLoopLast() const;
        s16 getLoopPenult() const;
        u32 getLoopStart() const;
        u32 getLoopEnd() const;
        u32 getNumSamples() const;

        u64 mOverallError = 0;
    };
    
    s32 decodeSample(u8 nibble, u8 scale, u8 index, s16 last, s16 penult);
    s32 convertPcmToNibble(s16 PCM, u8 scale, u8 index, s16 last, s16 penult);
    bool isValidS16Value(s32);
    u32 getADPCMBufferSize(u32 numSamples);
}