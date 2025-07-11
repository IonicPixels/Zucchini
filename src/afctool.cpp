// version 1.0

#include <ios>
#include <cmath>
#include <climits>
#include <cstring>
#include "include/afctool.h"



bool AFC::ADPCM_INFO::setInputPCMData(const BinaryReader& rIn, u32 dataLen) {
    if (dataLen % 2 != 0) {
        printf("Input file has an illegal file length!\n");
        return false;
    }
    
    mNumSamples = dataLen / 2;
    s16* pcmData = new s16 [mNumSamples];

    for (u32 i = 0; i < mNumSamples; i++)
        pcmData[i] = rIn.readS16LE(rIn.mCurrentPos + i * sizeof(s16));
    
    mPCM = pcmData;
    mSourceNumSamples = mNumSamples;
    return true;
}

void AFC::ADPCM_INFO::setInputPCMData(s16* pcmData, u32 numSamples) {
    mPCM = pcmData;
    mNumSamples = numSamples;
    mSourceNumSamples = numSamples;
}

bool AFC::ADPCM_INFO::setInputADPCMData(const BinaryReader& rIn, u32 dataLen) {
    if (dataLen % BYTES_PER_FRAME != 0) {
        printf("Input file has an illegal file length!\n");
        return false;
    }

    mNumSamples = (u32)(dataLen + (BYTES_PER_FRAME - 1)) / (u32)BYTES_PER_FRAME * (u32)SAMPLES_PER_FRAME;
    u8* adpcmData = new u8 [dataLen];

    for (u32 i = 0; i < dataLen; i++)
        adpcmData[i] = rIn.readU8(rIn.mCurrentPos + i * sizeof(u8));
    
    mADPCM = adpcmData;
    mSourceNumSamples = mNumSamples;
    return true;
}

void AFC::ADPCM_INFO::setLoopData(u32 loopStart, u32 loopEnd, bool requireCoef0AtLoopStart) {
    if (loopStart >= loopEnd || loopStart >= mNumSamples) {
        printf("WARNING! Invalid LoopStartSample was given!\nMake sure it's lower than the LoopEndSample.\nLoopStartSample set to 0...\n");
        mLoopStart = 0;
    }
    else
        mLoopStart = loopStart;

    mLoopLast = 0;
    mLoopPenult = 0;
    mIsLooped = true;
    mIsRequire0CoefAtLoop = requireCoef0AtLoopStart;
    mSourceLoopStart = mLoopStart;

    // fix loop points
    if (mLoopStart % SAMPLES_PER_FRAME != 0) {
        printf("Adjusting loop points...\n");

        mLoopStart = (u32)(mLoopStart + (SAMPLES_PER_FRAME - 1)) / (u32)SAMPLES_PER_FRAME * (u32)SAMPLES_PER_FRAME;
        u32 difference = mLoopStart - mSourceLoopStart; // calc difference
        loopEnd += difference;
        mNumSamples += difference;

        printf("NOTICE! The loop points were shifted:\n");
    }

    if (loopEnd >= mNumSamples)
        mLoopEnd = mNumSamples;
    else  {
        mNumSamples = loopEnd;
        mLoopEnd = loopEnd;
    }
    
    printf("Loop Start: %u, Loop End: %u\n", getLoopStart(), getLoopEnd());
}

u8* AFC::ADPCM_INFO::createADPCMBuffer() const {
    if (mNumSamples > 0)
        return new u8 [getADPCMBufferSize()]; // add an extra frames for loop alignment
    else
        return 0;
}

u32 AFC::ADPCM_INFO::getADPCMBufferSize() const {
    return AFC::getADPCMBufferSize(mNumSamples);
}

s16* AFC::ADPCM_INFO::createPCMBuffer() const {
    return new s16 [mNumSamples];
}

u32 AFC::ADPCM_INFO::getPCMBufferSize() const {
    return mNumSamples * sizeof(s16);
}

void AFC::ADPCM_INFO::encode(u8* dst) {
    printEncodeInfoStart();
    encode(dst, 0, mNumSamples);
    printEncodeInfoEnd();
}

void AFC::ADPCM_INFO::encode(u8* dst, u32 startSample, u32 numSamples) {
    if (!mPCM) {
        printf("No input PCM data set.\n");
        return;
    }

    if (startSample % SAMPLES_PER_FRAME != 0) {
        printf("The start sample needs to be a multiple of 16!\n");
        return;
    }

    // init
    if (startSample == 0) {
        mLast = 0;
        mPenult = 0;
    }

    numSamples += startSample;

    if (numSamples > mNumSamples)
        numSamples = mNumSamples;

    for (u32 i = startSample; i < numSamples; i += SAMPLES_PER_FRAME) {
        encodeFrame(dst, i);
        dst += BYTES_PER_FRAME;
    }
}

void AFC::ADPCM_INFO::printEncodeInfoStart() const {
    printf("Number of samples to encode: %u\n", getNumSamples());
}

void AFC::ADPCM_INFO::printEncodeInfoEnd() const {
    printf("\nOverall error in conversion: %llu\nAverage error per sample: %f\n\n", mOverallError, (f64)mOverallError / (f64)getNumSamples());
}

void AFC::ADPCM_INFO::decode(s16* dst) {
    if (!mADPCM) {
        printf("No input ADPCM data set.\n");
        return;
    }

    printf("Number of samples to decode: %u\n", getNumSamples());
    
    s16 pcm[SAMPLES_PER_FRAME];
    u32 bufferPos = 0;

    // init
    mLast = 0;
    mPenult = 0;
    const u8* adpcm = mADPCM;

    for (u32 i = 0; i < mNumSamples; i += SAMPLES_PER_FRAME) {
        decodeFrame(adpcm, pcm);
        adpcm += BYTES_PER_FRAME;

        for (u32 s = 0; s < SAMPLES_PER_FRAME && i + s < mNumSamples; s++)
            dst[bufferPos++] = pcm[s];
    }
}

s16 AFC::ADPCM_INFO::getLast() const {
    return mLast;
}

s16 AFC::ADPCM_INFO::getPenult() const {
    return mPenult;
}

s16 AFC::ADPCM_INFO::getLoopLast() const {
    return mLoopLast;
}

s16 AFC::ADPCM_INFO::getLoopPenult() const {
    return mLoopPenult;
}

u32 AFC::ADPCM_INFO::getLoopStart() const {
    return mLoopStart;
}

u32 AFC::ADPCM_INFO::getLoopEnd() const {
    return mLoopEnd;
}

u32 AFC::ADPCM_INFO::getNumSamples() const {
    return mNumSamples;
}

// from TLoZ Wind Waker's DecodeADPCM function
inline s32 AFC::decodeSample(u8 nibble, u8 scale, u8 index, s16 last, s16 penult) {
    return (sAdpcmNibbleToInt[nibble] << scale) + ((sAdpcmCoefficents[index][0] * last + sAdpcmCoefficents[index][1] * penult) >> 11);
}

inline s32 AFC::convertPcmToNibble(s16 PCM, u8 scale, u8 index, s16 last, s16 penult) {
    s32 scaleVal = (1 << scale);
    s32 coeffVal = (PCM - ((sAdpcmCoefficents[index][0] * last + sAdpcmCoefficents[index][1] * penult) >> 11));

    f64 result = (f64)(coeffVal) / (f64)(scaleVal);

    // help with rounding
    if (result < 0.0)
        result -= 0.5;
    else if (result > 0.0)
        result += 0.5;

    return (s32)(result);
}

inline bool AFC::isValidS16Value(s32 i) {
    if (i > INT16_MAX)
        return false;
    else if (i < INT16_MIN)
        return false;
    else
        return true;
}

void AFC::ADPCM_INFO::encodeFrame(u8 dst[BYTES_PER_FRAME], u32 currSamp) {
    u8 nibble[SAMPLES_PER_FRAME] = { 0 }; // 4-bit nibble
    u32 samples = (mNumSamples - currSamp >= SAMPLES_PER_FRAME) ? SAMPLES_PER_FRAME : mNumSamples - currSamp;
    u64 leastDiff = UINT64_MAX;
    s16 leastLast = 0;
    s16 leastPenalt = 0;
    u8 leastIndex = 0;
    u8 leastScale = 0;
    u32 ForceCoef0 = 16;
    s16 pcm[SAMPLES_PER_FRAME];

    if (currSamp >= mNumSamples) // don't decode 0 left samples
        return;

    // init pcm data
    u32 originalLoopStart = mSourceLoopStart;
    for (u32 s = 0; s < samples; s++) {
        if (currSamp + s < mSourceNumSamples)
            pcm[s] = mPCM[currSamp + s];
        else if (mIsLooped)
            pcm[s] = mPCM[originalLoopStart++];
        else
            pcm[s] = 0;
    }

    if (mIsLooped && currSamp == mLoopStart) {
        mLoopLast = mLast;
        mLoopPenult = mPenult;

        if (mIsRequire0CoefAtLoop) // force coef0 at loopStart for AST only
            ForceCoef0 = 1;
    }

    if (currSamp == 0) // always force coef0 at start
        ForceCoef0 = 1;

    // create coefs
    for (u32 i = 0; i < ForceCoef0; i++) {

        // create scale
        for (u32 c = 0; c < 16; c++) {
            s16 penult = mPenult;
            s16 last = mLast;
            u64 diff = 0;
            u8 nibble_tmp[SAMPLES_PER_FRAME];
            
            // create sample nibbles
            for (u32 s = 0; s < samples; s++) {
                s32 decSample_nib = 0;
                u64 leastError_nib = UINT64_MAX;

                // quickly predict the best nibble to be used
                s32 nibblePredict = convertPcmToNibble(pcm[s], c, i, last, penult);

                if (nibblePredict > 7)
                    nibblePredict = 7;
                else if (nibblePredict < -8)
                    nibblePredict = -8;
                
                u8 nibble_U8 = (u8)(nibblePredict & 0xF); // convert nibble to 4-bit
                decSample_nib = decodeSample(nibble_U8, c, i, last, penult);

                if (isValidS16Value(decSample_nib)) {
                    leastError_nib = llabs(decSample_nib - pcm[s]);
                    nibble_tmp[s] = nibble_U8;
                }
                else {
                    // if the prediction failed, find next valid nibble
                    for (u32 nib = 0; nib < 16; nib++) {
                        s32 dec_sample = decodeSample(nib, c, i, last, penult);
                        
                        if (isValidS16Value(dec_sample)) {
                            u64 nibDiff = llabs(dec_sample - pcm[s]);
                            
                            if (nibDiff < leastError_nib) {
                                leastError_nib = nibDiff;
                                nibble_tmp[s] = nib;
                                decSample_nib = dec_sample;
                            }
                        }
                    }

                    if (leastError_nib == UINT64_MAX) {
                        diff = UINT64_MAX; // did not find a valid nibble for this coef and scale
                        break;
                    }
                }

                diff += leastError_nib;
                penult = last;
                last = decSample_nib;
            }

            if (diff < leastDiff) {
                leastDiff = diff;
                leastLast = last;
                leastPenalt = penult;
                leastIndex = i;
                leastScale = c;

                for (u32 nibiter = 0; nibiter < samples; nibiter++)
                    nibble[nibiter] = nibble_tmp[nibiter];
            }
        }
    }

    if (leastDiff == UINT64_MAX) {
        printf("FATAL ERROR! No valid adpcm data could be created for the sample frame %d - %d\nTry decreasing the noise or volume of the sound wave and try again.\n", currSamp, currSamp +  SAMPLES_PER_FRAME);
        exit(1);
        return;
    }

    mOverallError += leastDiff;

    // write header
    dst[0] = (leastScale << 4) | (leastIndex & 0xF);

    // fill nibble data
    for (u32 i = 0; i < (BYTES_PER_FRAME - 1); i++)
        dst[i + 1] = ((nibble[i * 2] << 4)) | (nibble[i * 2 + 1] & 0xF);

    mLast = leastLast;
    mPenult = leastPenalt;
}



// this technically gives you the MOST accurate result you could ever get, but is sadly EXTREMELY slow
// it would probably take days to finish one conversion

/*void AFC::ADPCM_INFO::encodeFrame(u8 dst[BYTES_PER_FRAME], u32 currSamp) {
    printf("\"rendering\" frame %d / %d\n", currSamp / SAMPLES_PER_FRAME, mNumSamples / SAMPLES_PER_FRAME);

    u8 samples = (mNumSamples - currSamp >= SAMPLES_PER_FRAME) ? SAMPLES_PER_FRAME : mNumSamples - currSamp;
    s16 lastStart = mLast;
    s16 penaltStart = mPenult;

    u64 leastDiff = UINT64_MAX;
    u8 leastDiffOut[9];
    bool isFoundLeast = false;

    for (s32 a = 0; a < 256; a++) {
        dst[0] = a;

        for (s32 b = 0; b < 256; b++) {
            dst[1] = b;

            for (s32 c = 0; c < 256; c++) {
                dst[2] = c;

                for (s32 d = 0; d < 256; d++) {
                    dst[3] = d;

                    for (s32 e = 0; e < 256; e++) {
                        dst[4] = e;

                        for (s32 f = 0; f < 256; f++) {
                            dst[5] = f;

                            for (s32 g = 0; g < 256; g++) {
                                dst[6] = g;

                                for (s32 h = 0; h < 256; h++) {
                                    dst[7] = h;

                                    for (s32 i = 0; i < 256; i++) {
                                        dst[8] = i;
                                        u64 diff = _UI64_MAX;
                                        u8 index = dst[0] & 0xF;
                                        u8 scale = dst[0] >> 4;
                                        s16 last = lastStart;
                                        s16 penult = penaltStart;

                                        for (s32 sa_iter = 0; sa_iter < samples; sa_iter++) {
                                            u8 nibble = (sa_iter % 2 == 0) ? dst[1 + sa_iter / 2] >> 4 : dst[1 + sa_iter / 2] & 0xF;

                                            s32 dec_samp = decodeSample(nibble, scale, index, last, penult);

                                            if (!isValidS16Value(dec_samp)) {
                                                diff = _UI64_MAX; // invalidate because it clips
                                                break; // break because this route one creates invalid output
                                            }

                                            u64 local_diff = llabs(dec_samp - mPCM[currSamp + sa_iter]); // calc difference

                                            if (sa_iter == 0) // calc difference rate for single scale
                                                diff = local_diff;
                                            else
                                                diff += local_diff;

                                            penult = last;
                                            last = dec_samp;
                                        }

                                        if (diff < leastDiff) {
                                            leastDiff = diff;
                                            isFoundLeast = true;
                                            memcpy(leastDiffOut, dst, BYTES_PER_FRAME);
                                            mPenult = penult;
                                            mLast = last;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!isFoundLeast) {
        printf("FATAL ERROR! No valid adpcm data could be created for the sample frame %d - %d\nTry decreasing the noise or volume of the sound wave and try again.\n", currSamp, currSamp +  SAMPLES_PER_FRAME);
        exit(1);
        return;
    }

    memcpy(dst, leastDiffOut, BYTES_PER_FRAME);
}*/

void AFC::ADPCM_INFO::decodeFrame(const u8 adpcm[BYTES_PER_FRAME], s16 dst[SAMPLES_PER_FRAME]) {
    u8 scale = adpcm[0] >> 4;
    u8 index = adpcm[0] & 0xF;

    for (s32 i = 0; i < (BYTES_PER_FRAME - 1); i++) {
        dst[i * 2] = decodeSample(adpcm[i + 1] >> 4, scale, index, mLast, mPenult);
        mPenult = mLast;
        mLast = dst[i * 2];

        dst[i * 2 + 1] = decodeSample(adpcm[i + 1] & 0xF, scale, index, mLast, mPenult);
        mPenult = mLast;
        mLast = dst[i * 2 + 1];
    }
}

u32 AFC::getADPCMBufferSize(u32 numSamples) {
    return (u32)(numSamples + (SAMPLES_PER_FRAME - 1)) / (u32)SAMPLES_PER_FRAME * (u32)BYTES_PER_FRAME;
}