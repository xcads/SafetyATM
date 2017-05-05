#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#define LeftToBit(x, y) (((x) << (y)) | ((x) >> (32 - (y))))
#define HighBitToLowBit(x) (x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)|(x>>24)
#define FF_1(x,y,z) ((x) ^ (y) ^ (z))
#define FF_2(x,y,z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG_1(x,y,z) ((x) ^ (y) ^ (z))
#define GG_2(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define P_0(x) ((x) ^ LeftToBit((x),9) ^ LeftToBit((x),17))
#define P_1(x) ((x) ^ LeftToBit((x),15) ^ LeftToBit((x),23))
#define T_1 0x79cc4519
#define T_2 0x7a879d8a
//uint8_t putMessage[64]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
struct state_hash
{
	uint32_t A,B,C,D,E,F,G,H;
	uint32_t putlen[2];
};
class SM3
{
private:
	uint32_t T[2];
	uint32_t W_0[68];
	uint32_t W_1[64];
	//struct state_hash md;
	void ExtendW(uint32_t *input);
public:
	SM3(void);
	~SM3(void);
	void putbit(state_hash *md,uint8_t *input,uint8_t *output,int len,int *outLen);
	void sm3_init(state_hash *md);
	void ConvertTo32(uint8_t *input,uint32_t *output,int inputLen,int *outputLen);
	void ConvertToByte(uint32_t *input,uint8_t *output,int inputLen,int *outputLen);
	void CF(state_hash *md,uint32_t *B);
	void encode(state_hash *md,uint8_t *message,uint32_t *ciphermessage,int len);
};

