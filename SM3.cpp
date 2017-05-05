#include "SM3.h"


SM3::SM3(void)
{
}


SM3::~SM3(void)
{
}
void SM3::sm3_init(state_hash *md)
{
	md->A=0x7380166f;
	md->B=0x4914b2b9;
	md->C=0x172442d7;
	md->D=0xda8a0600;
	md->E=0xa96f30bc;
	md->F=0x163138aa;
	md->G=0xe38dee4d;
	md->H=0xb0fb0e4e;
	md->putlen[0]=0;
	md->putlen[1]=0;
}
void SM3::ConvertTo32(uint8_t *input,uint32_t *output,int inputLen,int *outputLen)
{
	*outputLen=0;
	for(int i = 0;i < inputLen; i+=4)
	{
		output[*outputLen] = (input[i] << 24) | (input[i+1] << 16) | (input[i+2] << 8) | input[i+3];
		(*outputLen)++;
	}
}
void SM3::ConvertToByte(uint32_t *input,uint8_t *output,int inputLen,int *outputLen)
{
	*outputLen=0;
	for(int i = 0; i < inputLen; i ++)
	{
		output[*outputLen + 3] = input[i] & 0xff;
		output[*outputLen + 2] = (input[i] >> 8) & 0xff;
		output[*outputLen + 1] = (input[i] >> 16) & 0xff;
		output[*outputLen] = (input[i] >> 24) & 0xff;
		*outputLen +=4;
	}
}
void SM3::ExtendW(uint32_t *input)
{
	for (int j = 0 ; j <68; j ++)
	{
		if(j < 16)
		{		
			W_0[j] = input[j];	
		}
		else
		{
			W_0[j]=P_1((W_0[j-16] ^ W_0[j-9] ^ LeftToBit(W_0[j-3],15))) ^ LeftToBit(W_0[j-13],7) ^ W_0[j-6]; 
		}
	}
	for (int j = 0;j < 64; j ++)
	{
		W_1[j]=W_0[j] ^ W_0[j+4];
	}
}
void SM3::putbit(state_hash *md,uint8_t *input,uint8_t *output,int len,int *outLen)
{
	uint8_t putMessage[64]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int k,l,tmp;
	uint8_t BitLen[8];
	k = len % 64;
	if ( k >56)
	{
		l = 120 - k;
	}
	else
	{
		l = 56 - k;
	}
	memcpy(output,input,len);
	memcpy(output+len,putMessage,l);
	*outLen = len + l;
	//md->putlen[1] = ((len * 8)>>32) & 0xffffffff;
	md->putlen[1] = (len * 8) & 0xffffffff;
	ConvertToByte(md->putlen,BitLen,2,&tmp);
	memcpy(output+len+l,BitLen,8);
	//memcpy(output+len+l,md->putlen+1,4);
	//memcpy(output+len+l+4,md->putlen,4);
	*outLen = *outLen + 8;
}
void SM3::CF(state_hash *md,uint32_t *B)
{
	uint32_t AA,BB,CC,DD,EE,FF,GG,HH,SS1,SS2,TT1,TT2;
	ExtendW(B);
	AA = md->A;
	BB = md->B;
	CC = md->C;
	DD = md->D;
	EE = md->E;
	FF = md->F;
	GG = md->G;
	HH = md->H;
	for (int j = 0;j < 64 ; j++)
	{
		if(j < 16)
		{
			SS1 = LeftToBit(LeftToBit(AA,12)+EE+LeftToBit(T_1,j),7);
			SS2 = SS1 ^ LeftToBit(AA,12);
			TT1 = FF_1(AA,BB,CC) + DD + SS2 + W_1[j];
			TT2 = GG_1(EE,FF,GG) + HH + SS1 + W_0[j];
		}
		else
		{
			SS1 = LeftToBit(LeftToBit(AA,12)+EE+LeftToBit(T_2,j),7);
			SS2 = SS1 ^ LeftToBit(AA,12);
			TT1 = FF_2(AA,BB,CC) + DD + SS2 + W_1[j];
			TT2 = GG_2(EE,FF,GG) + HH + SS1 + W_0[j];
		}
		DD = CC;
		CC = LeftToBit(BB,9);
		BB = AA;
		AA = TT1;
		HH = GG;
		GG = LeftToBit(FF,19);
		FF = EE;
		EE = P_0(TT2);
	}
	md->A = AA ^ md->A;
	md->B = BB ^ md->B;
	md->C = CC ^ md->C;
	md->D = DD ^ md->D;
	md->E = EE ^ md->E;
	md->F = FF ^ md->F;
	md->G = GG ^ md->G;
	md->H = HH ^ md->H;
}
void SM3::encode(state_hash *md,uint8_t *message,uint32_t *ciphermessage,int len)
{
	//uint8_t putMessage[64]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t *tmpMessage;
	int RealLen;
	uint32_t B[16];
	int bLen;
	tmpMessage = (uint8_t *)malloc(sizeof(uint8_t)*(len+120));
	memset(tmpMessage,0,len+120);
	putbit(md,message,tmpMessage,len, &RealLen);
	for (int i = 0; i <RealLen / 64; i++)
	{
		ConvertTo32(tmpMessage + i * 64,B,64,&bLen);
		CF(md,B);
	}
	ciphermessage[0]=md->A;
	ciphermessage[1]=md->B;
	ciphermessage[2]=md->C;
	ciphermessage[3]=md->D;
	ciphermessage[4]=md->E;
	ciphermessage[5]=md->F;
	ciphermessage[6]=md->G;
	ciphermessage[7]=md->H;
	delete(tmpMessage);
}
