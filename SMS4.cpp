#include "SMS4.h"


SMS4::SMS4()
{
	memset(CX, 0, sizeof(uint32_t) * 36);
	memset(MK, 0, sizeof(uint32_t) * 4);
	memset(rk, 0, sizeof(uint32_t) * 32);
	memset(CK, 0, sizeof(uint32_t) * 32);
	memset(ck, 0, sizeof(uint32_t) * 32);
}


SMS4::~SMS4()
{
}
void SMS4::SMS4_init()
{
	memset(CX, 0, sizeof(uint32_t) * 36);
	memset(MK, 0, sizeof(uint32_t) * 4);
	memset(rk, 0, sizeof(uint32_t) * 32);
	memset(CK, 0, sizeof(uint32_t) * 32);
	memset(ck, 0, sizeof(uint32_t) * 32);
}
uint32_t SMS4::XunConvert(uint32_t x, uint8_t n)
{
	return ((x << n) | (x >> (32 - n)));
}
void SMS4::ProductFk()
{
	FK[0] = 0xa3b1bac6;
	FK[1] = 0x56aa3350;
	FK[2] = 0x677d9197;
	FK[3] = 0xb27022dc;
}
void SMS4::ProductCK()
{
	int i, j;
	uint8_t ckJ[4];
	for (i = 0; i < 32; i++)
	{
		for (j = 0; j < 4; j++)
		{
			ckJ[j] = ((4 * i + j) * 7) % 256;
		}
		CK[i] = (((ckJ[0] << 8) << 8) << 8) | ((ckJ[1] << 8) << 8) | (ckJ[2] << 8) | ckJ[3];
	}
}
uint32_t SMS4::Sbox(uint32_t x)
{
	uint8_t *px = (uint8_t *)&x;
	px[0] = S[(px[0] >> 4) & 0x0f][px[0] & 0x0f];
	px[1] = S[(px[1] >> 4) & 0x0f][px[1] & 0x0f];
	px[2] = S[(px[2] >> 4) & 0x0f][px[2] & 0x0f];
	px[3] = S[(px[3] >> 4) & 0x0f][px[3] & 0x0f];
	return x;
}
uint32_t SMS4::LDconvert(uint32_t x)
{
	return (x^XunConvert(x, 13) ^ XunConvert(x, 23));
}
uint32_t SMS4::LEconvert(uint32_t x)
{
	return (x^XunConvert(x, 2) ^ XunConvert(x, 10) ^ XunConvert(x, 18) ^ XunConvert(x, 24));
}
uint32_t SMS4::TDconvert(uint32_t x)
{
	return LDconvert(Sbox(x));
}
uint32_t SMS4::TEconvert(uint32_t x)
{
	return LEconvert(Sbox(x));
}
void SMS4::ProductRk()
{
	uint32_t K[36];

	int i;
	ProductFk();
	ProductCK();
	for (i = 0; i < 36; i++)
	{
		if (i < 4)
		{
			K[i] = MK[i] ^ FK[i];
		}
		else
		{
			rk[i - 4] = K[i] = K[i - 4] ^ TDconvert(K[i - 3] ^ K[i - 2] ^ K[i - 1] ^ CK[i - 4]);
		}
	}
}
void SMS4::Encode(uint32_t *Message, uint32_t *Key, uint32_t *cipherMessage,int len)
{
	int i, j, k;
	for (i = 0; i < 4; i++)
	{
		MK[i] = Key[i];
	}
	ProductRk();
	for (k = 0; k <= len; k++)
	{
		if (k != 0 && k % 4 == 0)
		{
			for (i = 0, j = k - 4; i < 4; i++,j++)
			{
				CX[i] = Message[j];
			}
			
			for (i = 4; i < 36; i++)
			{
				CX[i] = CX[i - 4] ^ TEconvert(CX[i - 1] ^ CX[i - 2] ^ CX[i - 3] ^ rk[i - 4]);//Xor(CX[i - 4], TEconvert(Xor(Xor(CX[i - 1], CX[i - 2]), Xor(CX[i - 3], rk[i - 4]))));
			}
			for (i = k - 4, j = 35; j > 31; i++, j--)
			{
				cipherMessage[i] = CX[j];
			}
		}
	}
}
void SMS4::Dncode(uint32_t *cipherMessage, uint32_t *Key, uint32_t *Message,int len)
{
	int i, j, k;
	for (i = 0; i < 4; i++)
	{
		MK[i] = Key[i];
	}
	ProductRk();
	for (k = 0; k <= len; k++)
	{
		if (k != 0 && k % 4 == 0)
		{
			for (i = 0, j = k - 4; i < 4; i++, j++)
			{
				CX[i] = cipherMessage[j];
			}
			for (i = 4, j = 35; i < 36; i++, j--)
			{
				CX[i] = CX[i - 4] ^ TEconvert(CX[i - 1] ^ CX[i - 2] ^ CX[i - 3] ^ rk[j - 4]);
			}
			for (i = k-4, j = 35; j > 31; i++, j--)
			{
				Message[i] = CX[j];
			}
		}
	}
}
void SMS4::ConvertTo32(uint32_t *output, uint8_t *input, uint32_t len, int *outLength)
{
	int i, j;
	for (i = 0, j = 0; j < len; j += 4, i++)
	{
		output[i] = (input[j + 3]) | (input[j + 2] << 8) | (input[j + 1] << 16) | (input[j] << 24);
	}
	*outLength = i;
}
