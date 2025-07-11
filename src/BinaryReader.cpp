// version 2.0


#include <iostream>

#include "include/BinaryReader.h"

bool BinaryReader::isSystemLittleEndian() {
    endianCheck checker;
    checker.mU16 = 0xFFEE;

    return ((u32)(checker.mU8[0] << 8) | (u32)checker.mU8[1]) != 0xFFEE;
}

BinaryReader::BinaryReader(const char* pName) : FileHolder(pName), mCurrentPos(0), mIsSystemLittleEndian(isSystemLittleEndian()), mHasReachedEndOfFile(false) {}

bool BinaryReader::doFileCheck(u32 pos) const {
    if (pos >= mFileSize) {
        std::cout << "WARNING: end of file was reached! (" << mFileName << ")\n";
        return false;
    }
    else
        return true;
}

bool BinaryReader::doFileCheckWithEndFlag(u32 pos) {
    bool i = doFileCheck(pos);
    mHasReachedEndOfFile = !i;
    return i;
}

u32 BinaryReader::readValueBE(u32 pos, s32 NumBytes) const {
    if (!doFileCheck(pos + NumBytes - 1))
        return 0;

    // byteswap + load val
    const u8* pCurrentPos = (u8*)(mFileData + pos);
    u32 val = 0;

    if (mIsSystemLittleEndian) {
        u32 f = 0;
        for (s32 i = NumBytes - 1; i >= 0; i--) {
            val |= pCurrentPos[i] << f * 8; // byteswap!
            f++;
        }
    }
    else {
        for (s32 i = 0; i < NumBytes; i++)
            val |= pCurrentPos[i] << i * 8;
    }

    return val;
}

u32 BinaryReader::readValueBE(s32 NumBytes) {
    if (!doFileCheckWithEndFlag(mCurrentPos + NumBytes - 1))
        return 0;

    u32 val = readValueBE(mCurrentPos, NumBytes);
    mCurrentPos += NumBytes;
    return val;
}

u8 BinaryReader::readU8(u32 pos) const {
    return readValueBE(pos, 1);
}

u16 BinaryReader::readU16BE(u32 pos) const {
    return readValueBE(pos, 2);
}

s16 BinaryReader::readS16BE(u32 pos) const {
    return (s16)readValueBE(pos, 2);
}

u32 BinaryReader::readU24BE(u32 pos) const {
    return readValueBE(pos, 3);
}

u32 BinaryReader::readU32BE(u32 pos) const {
    return readValueBE(pos, 4);
}

s32 BinaryReader::readS32BE(u32 pos) const {
    return (s32)readValueBE(pos, 4);
}

// read from CurrentPos and advances
u8 BinaryReader::readU8() {
    return readValueBE(1);
}

u16 BinaryReader::readU16BE() {
    return readValueBE(2);
}

s16 BinaryReader::readS16BE() {
    return (s16)readValueBE(2);
}

u32 BinaryReader::readU24BE() {
    return readValueBE(3);
}

u32 BinaryReader::readU32BE() {
    return readValueBE(4);
}

s32 BinaryReader::readS32BE() {
    return (s32)readValueBE(4);
}

////////////////// LITTLE ENDIAN //////////////////
u32 BinaryReader::readValueLE(u32 pos, s32 NumBytes) const {
    if (!doFileCheck(pos + NumBytes - 1))
        return 0;

    // byteswap + load val
    const u8* pCurrentPos = (u8*)(mFileData + pos);
    u32 val = 0;

    if (!mIsSystemLittleEndian) {
        u32 f = 0;
        for (s32 i = NumBytes - 1; i >= 0; i--) {
            val |= pCurrentPos[i] << f * 8; // byteswap!
            f++;
        }
    }
    else {
        for (s32 i = 0; i < NumBytes; i++)
            val |= pCurrentPos[i] << i * 8;
    }

    return val;
}

u32 BinaryReader::readValueLE(s32 NumBytes) {
    if (!doFileCheckWithEndFlag(mCurrentPos + NumBytes - 1))
        return 0;

    u32 val = readValueLE(mCurrentPos, NumBytes);
    mCurrentPos += NumBytes;
    return val;
}

u16 BinaryReader::readU16LE(u32 pos) const {
    return readValueLE(pos, 2);
}

s16 BinaryReader::readS16LE(u32 pos) const {
    return (s16)readValueLE(pos, 2);
}

u32 BinaryReader::readU24LE(u32 pos) const {
    return readValueLE(pos, 3);
}

u32 BinaryReader::readU32LE(u32 pos) const {
    return readValueLE(pos, 4);
}

s32 BinaryReader::readS32LE(u32 pos) const {
    return (s32)readValueLE(pos, 4);
}

u16 BinaryReader::readU16LE() {
    return readValueLE(2);
}

s16 BinaryReader::readS16LE() {
    return (s16)readValueLE(2);
}

u32 BinaryReader::readU24LE() {
    return readValueLE(3);
}

u32 BinaryReader::readU32LE() {
    return readValueLE(4);
}

s32 BinaryReader::readS32LE() {
    return (s32)readValueLE(4);
}





void BinaryReader::writeValueBE(u32 pos, s32 NumBytes, u32 val) {
    if (!mFileData || !doFileCheck(pos + NumBytes - 1))
        return;

    // byteswap + write val
    u8* pCurrentPos = (u8*)(mFileData + pos);

    if (mIsSystemLittleEndian) {
        u32 f = 0;
        for (s32 i = NumBytes - 1; i >= 0; i--) {
            pCurrentPos[i] = (val >> f * 8) & 0b11111111;
            f++;
        }
    }
    else {
        for (s32 i = 0; i < NumBytes; i++)
            pCurrentPos[i] = (val >> i * 8) & 0b11111111;
    }
}

void BinaryReader::writeValueBE(s32 NumBytes, u32 val) {
    if (!mFileData) {
        mCurrentPos += NumBytes;
        return;
    }

    if (!doFileCheckWithEndFlag(mCurrentPos + NumBytes - 1))
        return;

    writeValueBE(mCurrentPos, NumBytes, val);
    mCurrentPos += NumBytes;
}

void BinaryReader::writeU8(u32 pos, u8 val) {
    writeValueBE(pos, 1, val);
}

void BinaryReader::writeU16BE(u32 pos, u16 val) {
    writeValueBE(pos, 2, val);
}

void BinaryReader::writeS16BE(u32 pos, s16 val) {
    writeValueBE(pos, 2, (u32)val);
}

void BinaryReader::writeU24BE(u32 pos, u32 val) {
    writeValueBE(pos, 3, val);
}

void BinaryReader::writeU32BE(u32 pos, u32 val) {
    writeValueBE(pos, 4, val);
}

void BinaryReader::writeS32BE(u32 pos, s32 val) {
    writeValueBE(pos, 4, (u32)val);
}

void BinaryReader::writeU8(u8 val) {
    writeValueBE(1, val);
}

void BinaryReader::writeU16BE(u16 val) {
    writeValueBE(2, val);
}

void BinaryReader::writeS16BE(s16 val) {
    writeValueBE(2, (u32)val);
}

void BinaryReader::writeU24BE(u32 val) {
    writeValueBE(3, val);
}

void BinaryReader::writeU32BE(u32 val) {
    writeValueBE(4, val);
}

void BinaryReader::writeS32BE(s32 val) {
    writeValueBE(4, (u32)val);
}