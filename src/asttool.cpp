// version 1.0

#include <iostream>
#include <cstring>
#include <unistd.h>
#include "include/asttool.h"

u32 AST::getNumBlckHeaders(u32 numSamples) {
    u32 adpcmBlockSize = AFC::getADPCMBufferSize(numSamples);

    while (adpcmBlockSize % 0x20) // align size by 0x20
        adpcmBlockSize++;

    return (u32)(adpcmBlockSize + (BLCK_BLOCK_SIZE - 1)) / (u32)BLCK_BLOCK_SIZE;
}

u32 AST::getBufferSize(u32 numSamples, u32 channels) {
    u32 numBlckHeaders = getNumBlckHeaders(numSamples);
    u32 size = STRM_HEADER_SIZE + numBlckHeaders * BLCK_HEADER_SIZE;

    // get BLCK block size
    u32 adpcmBlockSize = AFC::getADPCMBufferSize(numSamples);

    while (adpcmBlockSize % 0x20) // align size by 0x20
        adpcmBlockSize++;

    if (adpcmBlockSize % BLCK_BLOCK_SIZE == 0) // all blocks are full
        return size + adpcmBlockSize * channels;
    else {
        u32 sizeOfFullBlocks = (numBlckHeaders - 1) * BLCK_HEADER_SIZE;
        u32 sizeOfLastBlock = adpcmBlockSize - sizeOfFullBlocks;

        return size + (sizeOfFullBlocks + sizeOfLastBlock) * channels;
    }
}

bool AST::makeAst(s16** in, BinaryReader& outFile, u32 numSamples, u32 channels, u32 samplerate, bool isLooped, u32 loopStart, u32 loopEnd) {
    AFC::ADPCM_INFO** channelADPCM = new AFC::ADPCM_INFO * [channels];
    u32 numAdpcmSamples = numSamples;

    // prepare AST and ADPCM
    for (u32 c = 0; c < channels; c++) {
        AFC::ADPCM_INFO* adpcm = new AFC::ADPCM_INFO();
        channelADPCM[c] = adpcm;

        adpcm->setInputPCMData(in[c], numSamples);

        if (isLooped)
            adpcm->setLoopData(loopStart, loopEnd, true);

        // make header
        if (c == 0) {
            outFile.mCurrentPos = 0;
            outFile.mFileSize = getBufferSize(adpcm->getNumSamples(), channels);
            outFile.mFileData = new char [outFile.mFileSize];
            memset(outFile.mFileData, 0, outFile.mFileSize);
            
            outFile.writeU32BE('STRM');
            outFile.writeU32BE(outFile.mFileSize - STRM_HEADER_SIZE);
            outFile.writeU16BE(0); // format: 0 = ADPCM, 1 = PCM16
            outFile.writeU16BE(16); // decode format? always 16-bit (PCM)
            outFile.writeU16BE(channels);
            outFile.writeU16BE(isLooped ? 0xFFFF : 0x0);
            outFile.writeU32BE(samplerate);
            outFile.writeU32BE(adpcm->getNumSamples());
            outFile.writeU32BE(isLooped ? adpcm->getLoopStart() : 0);
            outFile.writeU32BE(isLooped ? adpcm->getLoopEnd() : adpcm->getNumSamples());
            outFile.writeU32BE(BLCK_BLOCK_SIZE); // BCLK block size
            outFile.writeU32BE(0); // padding?
            outFile.writeU8(127); // volume?

            while (outFile.mCurrentPos < STRM_HEADER_SIZE) // add padding
                outFile.writeU8(0);

            numAdpcmSamples = adpcm->getNumSamples();
        }
    }

    u32 numBLCK = getNumBlckHeaders(numAdpcmSamples);
    u32 currAdpcmSample = 0;

    for (u32 i = 0; i < numBLCK; i++) {
        u32 bytesToEnd = numAdpcmSamples - currAdpcmSample;

        if (bytesToEnd > ADPCM_SAMPLES_PER_BLOCK)
            bytesToEnd = ADPCM_SAMPLES_PER_BLOCK;

        bytesToEnd = (u32)(bytesToEnd + (SAMPLES_PER_FRAME - 1)) / (u32)SAMPLES_PER_FRAME * (u32)BYTES_PER_FRAME; // convert samples to adpcm bytes

        // align
        while (bytesToEnd % 0x20) // add padding
            bytesToEnd++;

        // write BLCK header
        outFile.writeU32BE('BLCK');
        outFile.writeU32BE(bytesToEnd); // BLCK block size
        u32 penultAndLastPos = outFile.mCurrentPos;
        outFile.mCurrentPos += 0x18;

        for (u32 c = 0; c < channels; c++) { // write data
            channelADPCM[c]->encode((u8*)outFile.mFileData + outFile.mCurrentPos, currAdpcmSample, ADPCM_SAMPLES_PER_BLOCK);
            outFile.mCurrentPos += bytesToEnd;
        }

        // write last and penult for each channel
        
        for (u32 c = 0; c < channels; c++) {
            outFile.writeS16BE(penultAndLastPos, channelADPCM[c]->getLast());
            outFile.writeS16BE(penultAndLastPos + 2, channelADPCM[c]->getPenult());
            penultAndLastPos += 4;
        }
        
        std::cout << "\rConverting to AST: " << (f64)currAdpcmSample / (f64)numAdpcmSamples * 100.0 << "%     " << std::flush;
        currAdpcmSample += ADPCM_SAMPLES_PER_BLOCK;
    }

    printf("\n\n");

    for (u32 c = 0; c < channels; c++) { // write info
        printf("Channel %d info:", c + 1);
        channelADPCM[c]->printEncodeInfoEnd();
    }

    return true;
}



bool AST::convertAstPcm16ToAstAdpcm(BinaryReader& inAst, BinaryReader& outAst) {
    // extract info from original AST
    inAst.mCurrentPos = 0;

    if (inAst.readU32BE() != 'STRM') { // header
        printf("Input file is not a valid AST file!\n");
        sleep(5);
        return false;
    }

    if (inAst.readU32BE() + 0x40 != inAst.mFileSize) { // total block size
        printf("Input AST seems to be corrupted!\n");
        sleep(5);
        return false;
    }

    if (inAst.readU16BE() != 1) { // encoding format
        printf("Input AST is already ADPCM encoded!\n");
        sleep(5);
        return false;
    }

    inAst.readU16BE(); // decoded bits per sample?

    u32 channels = inAst.readU16BE();

    if (channels > 6) { // channel num
        printf("Input AST has more than 6 channel thus it seems to invalid.\n");
        sleep(5);
        return false;
    }

    bool isLooped = (inAst.readU16BE() == 0xFFFF);
    u32 sampleRate = inAst.readU32BE();
    u32 numSamples = inAst.readU32BE();
    u32 loopStart = inAst.readU32BE();
    u32 loopEnd = inAst.readU32BE();

    inAst.readU32BE(); // block size
    inAst.readU32BE(); // unk
    inAst.readU8(); // volume?

    inAst.mCurrentPos = 0x40;

    if (numSamples < loopEnd)
        numSamples = loopEnd;

    // extract PCM16 data
    s16** pPCM = new s16 * [channels];

    for (u32 c = 0; c < channels; c++)
        pPCM[c] = new s16 [numSamples + 32]; // add a bit of extra buffer

    for (u32 currSamp = 0; currSamp < numSamples;) {
        if (inAst.readU32BE() != 'BLCK') { // header string
            printf("Input AST is corrupted!\n");
            sleep(5);
            return false;
        }

        u32 blockSize = inAst.readU32BE();
        inAst.mCurrentPos += 0x18;

        if (blockSize % 2) {
            printf("Input AST is corrupted!\n");
            sleep(5);
            return false;
        }

        blockSize /= 2;

        for (u32 c = 0; c < channels; c++) {
            for (u32 parser = 0; parser < blockSize && currSamp + parser < numSamples; parser++)
                pPCM[c][currSamp + parser] = inAst.readS16BE();
        }
        
        currSamp += blockSize;
    }

    makeAst(pPCM, outAst, numSamples, channels, sampleRate, isLooped, loopStart, loopEnd);
    return true;
}