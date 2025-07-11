// version 1.0

#include <ios>
#include "include/wavfile.h"


bool WAV::WAV_INFO::parseFile() {
    mCurrentPos = 0;

    // parse RIFF
    if (readU32BE() != 'RIFF' || readU32LE() == 0 || readU32BE() != 'WAVE') {
        printf("Input file is not a valid WAV file!\n");
        return false;
    }

    // find fmt chunk
    bool foundFmt = false;
    bool foundDat = false;

    while (mCurrentPos < mFileSize) {
        u32 pos = mCurrentPos;
        u32 fmt = readU32BE();
        u32 nextChunkPos = readU32LE() + mCurrentPos; // read format chunk size
        mCurrentPos = pos;

        switch (fmt) {
            case 'fmt ': {
                foundFmt = true;

                if (!parseFmt())
                    return false;
                
                break;
            }

            case 'data': {
                foundDat = true;

                if (!parseData())
                    return false;
                
                break;
            }

            case 'smpl': {
                if (!parseSmpl())
                    return false;
                
                break;
            }

            default: {
                printf("WARNING: Unsupported chunk \"%c%c%c%c\" found in WAV file at 0x%X! Ignoring chunk...\n", fmt >> 24, fmt >> 16, fmt >> 8, fmt, pos);
                break;
            }
        }

        mCurrentPos = nextChunkPos;
    }

    if (!foundFmt || !foundDat) {
        printf("Input file does not contain necessary audio data!\n");
        return false;
    }

    // do final stuff
    if (mNumSamples % (2 * mChannelCount) != 0)  {
        printf("Input WAV is invalid!\n");
        return false;
    }

    mNumSamples /= 2 * mChannelCount;
    
    return true;
}

bool WAV::WAV_INFO::parseFmt() {
    readU32BE(); // chunk magic, ignoring
    readU32LE(); // chunk size, ignoring

    if (readU16LE() != 1) {
        printf("Input WAV is not PCM!\n");
        return false;
    }

    mChannelCount = readU16LE();
    mSampleRate = readU32LE();
    readU32LE(); // Byte rate, what do i need this for?
    
    readU16LE(); // check for align block skipped, because some exporters don't get it right?
    /*if (readU16LE() != mChannelCount * 2) {
        printf("Input WAV is invalid!\n");
        return false;
    }*/

    if (readU16LE() != 16) {
        printf("Input WAV is not 16-bit PCM!\n");
        return false;
    }

    return true;
}

bool WAV::WAV_INFO::parseData() {
    readU32BE(); // chunk magic, ignoring
    mNumSamples = readU32LE(); // chunk size
    mDataChunkOfs = mCurrentPos;

    return true;
}

bool WAV::WAV_INFO::parseSmpl() {
    readU32BE(); // chunk magic, ignoring
    readU32LE(); // chunk size, ignoring

    readU32LE(); //_0 dwManufacturer
    readU32LE(); //_4 dwProduct
    readU32LE(); //_8 dwSamplePeriod
    readU32LE(); //_C dwMIDIUnityNote
    readU32LE(); //_10 dwMIDIPitchFraction
    readU32LE(); //_14 dwSMPTEFormat
    readU32LE(); //_18 dwSMPTEOffset
    u32 loopNum = readU32LE(); //_1C cSampleLoops
    readU32LE(); //_20 cbSamplerData

    if (loopNum > 0) {
        if (loopNum > 1)
            printf("WARNING: The WAV file has more than one loop point! Only the first loop will be parsed.\n");
        
        mHasLoop = true;
        readU32LE(); //_0 dwIdentifier
        readU32LE(); //_4 dwType

        mLoopStart = readU32LE(); //_8 dwStart
        mLoopEnd = readU32LE(); //_C dwEnd

        if (mLoopStart >= mLoopEnd) {
            printf("ERROR: The LoopStart of the WAV is greater than the LoopEnd!\n");
            return false;
        }

        // the rest doesn't matter
        // readU32LE(); //_10 dwFraction
        // readU32LE(); //_14 dwPlayCount
    }

    return true;
}

s16** WAV::WAV_INFO::makeWaveDataBuffers() {
    if (mChannelCount == 0) {
        printf("No channels found in WAV file!\n");
        return 0;
    }

    s16** out = new s16 * [mChannelCount];

    for (u32 i = 0; i < mChannelCount; i++)
        out[i] = new s16 [mNumSamples];

    u32 pos = mCurrentPos;
    mCurrentPos = mDataChunkOfs;

    // split and copy wave data
    for (u32 i = 0; i < mNumSamples; i++) {
        for (u32 c = 0; c < mChannelCount; c++)
                out[c][i] = readS16LE();
    }

    mCurrentPos = pos;

    return out;
}

u32 WAV::WAV_INFO::getWaveDataSize() const {
    return mNumSamples * mChannelCount * 2;
}

u32 WAV::WAV_INFO::getNumSamples() const {
    return mNumSamples;
}

u32 WAV::WAV_INFO::getNumChannels() const {
    return mChannelCount;
}

u32 WAV::WAV_INFO::getSamplerate() const {
    return mSampleRate;
}

bool WAV::WAV_INFO::isLooped() const {
    return mHasLoop;
}

u32 WAV::WAV_INFO::getLoopStartSample() const {
    if (!mHasLoop || mLoopStart >= mNumSamples)
        return 0;
    else
        return mLoopStart;
}

u32 WAV::WAV_INFO::getLoopEndSample() const {
    if (!mHasLoop || mLoopEnd >= mNumSamples)
        return mNumSamples;
    else
        return mLoopEnd;
}