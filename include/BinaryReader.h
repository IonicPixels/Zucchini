// version 2.0


#pragma once

#include "include/FileHolder.h"

class BinaryReader : public FileHolder {
private:
    bool doFileCheck(u32) const;
    bool doFileCheckWithEndFlag(u32);
    static bool isSystemLittleEndian();

    union endianCheck {
        u16 mU16;
        u8 mU8[2];
    };

public:
    inline BinaryReader() : mCurrentPos(0), mIsSystemLittleEndian(isSystemLittleEndian()), mHasReachedEndOfFile(false) {}
    inline BinaryReader(char* pDat, size_t len) : mCurrentPos(0), mIsSystemLittleEndian(isSystemLittleEndian()), mHasReachedEndOfFile(false) { mFileData = pDat; mFileSize = len; }
    BinaryReader(const char*);

    inline void resetPosition() {
        mCurrentPos = 0;
        mHasReachedEndOfFile = false;
    }

    inline void setPosition(u32 pos) {
        mCurrentPos = pos;
        mHasReachedEndOfFile = !doFileCheck(pos);
    }

    u32 readValueBE(u32 pos, s32 NumBytes) const;
    u32 readValueBE(s32 NumBytes);
    u32 readValueLE(u32 pos, s32 NumBytes) const;
    u32 readValueLE(s32 NumBytes);

    u8 readU8(u32 pos) const;
    u16 readU16BE(u32 pos) const;
    s16 readS16BE(u32 pos) const;
    u32 readU24BE(u32 pos) const;
    u32 readU32BE(u32 pos) const;
    s32 readS32BE(u32 pos) const;
    u16 readU16LE(u32 pos) const; // little endian
    s16 readS16LE(u32 pos) const;
    u32 readU24LE(u32 pos) const;
    u32 readU32LE(u32 pos) const;
    s32 readS32LE(u32 pos) const;

    // read from CurrentPos and advances
    u8 readU8();
    u16 readU16BE();
    s16 readS16BE();
    u32 readU24BE();
    u32 readU32BE();
    s32 readS32BE();
    u16 readU16LE(); // little endian
    s16 readS16LE();
    u32 readU24LE();
    u32 readU32LE();
    s32 readS32LE();

    // write! (even tho this is called a Reader)
    void writeValueBE(u32 pos, s32 NumBytes, u32);
    void writeValueBE(s32 NumBytes, u32);

    void writeU8(u32 pos, u8);
    void writeU16BE(u32 pos, u16);
    void writeS16BE(u32 pos, s16);
    void writeU24BE(u32 pos, u32);
    void writeU32BE(u32 pos, u32);
    void writeS32BE(u32 pos, s32);

    void writeU8(u8);
    void writeU16BE(u16);
    void writeS16BE(s16);
    void writeU24BE(u32);
    void writeU32BE(u32);
    void writeS32BE(s32);

    u32 mCurrentPos;
    bool mIsSystemLittleEndian;
    bool mHasReachedEndOfFile;
};