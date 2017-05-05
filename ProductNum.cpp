#include "ProductNum.h"


ProductNum::ProductNum()
{
}


ProductNum::~ProductNum()
{
}
void ProductNum::numPro(uint8_t *s)
{

	for (int i = 0; i < 8; i++)
		s[i] = rand() % 10 + '0';
}
