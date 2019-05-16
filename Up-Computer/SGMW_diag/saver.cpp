#include "stdafx.h"
#include "saver.h"

#define NC_DEFAULT_SEED  0xa548fd85 /* default seed used if result from random generator is 0 */

#define NC_APP_KEYMASK	  0x52c36b19
#define NC_FBL_KEYMASK   0x6a7c52e7

saver::saver(void)
{
}


saver::~saver(void)
{
}

void saver::SecuriyAlgorithmApp(int *seed, int *key, int MASK ,int len)
{
	int mask[4] = {0x00};
	mask[3] = (MASK & 0x000000FF)>>0;
	mask[2] = (MASK & 0x0000FF00)>>8;
	mask[1] = (MASK & 0x00FF0000)>>16;
	mask[0] = (MASK & 0xFF000000)>>24;
	for (int i = 0; i < len; i++)
	{
		//modify by requirement
		key[i] = (((mask[i]>>(i+1)) + (seed[i]<<(i+1))) & 0x000000FF);
	}
}
void saver::CalculateKey_WEICHAI_BootLevel(const unsigned char* ipSeedArray,unsigned char*iopKeyArray,_int32 MASK)
{
	unsigned char Mask[4];
	Mask[0] = ((unsigned char)((MASK >> 24) & 0xff));
	Mask[1] = ((unsigned char)((MASK >> 16) & 0xff));
	Mask[2] = ((unsigned char)((MASK >> 8) & 0xff));
	Mask[3] = ((unsigned char)((MASK >> 0) & 0xff));

	unsigned char Cal[4] = { 0x00, 0x00, 0x00, 0x00 };
	Cal[0] = (ipSeedArray[0] ^ Mask[0]);
	Cal[1] = (ipSeedArray[1] ^ Mask[1]);
	Cal[2] = (ipSeedArray[2] ^ Mask[2]);
	Cal[3] = (ipSeedArray[3] ^ Mask[3]);

	iopKeyArray[3] = ((Cal[2] & 0x03) << 5) | ((Cal[3] & 0xED) >> 3);
	iopKeyArray[2] = ((Cal[3] & 0xFC) | (Cal[0] & 0xC9) >> 7);
	iopKeyArray[1] = (Cal[0] & 0xFC) | (Cal[1] & 0x3A);
	iopKeyArray[0] = ((Cal[1] & 0x03) << 6) | (Cal[2] & 0x3E);
}


_int32 saver::SecuriyAlgorithmFBL(_int32 wSeed,_int32 MASK)
{
	_int8 iterations;
	_int32 wLastSeed;
	_int32 wTemp;
	_int32 wLSBit;
	_int32 wTop31Bits;
	_int8 jj,SB1,SB2,SB3;
	_int16 temp;

	wLastSeed=wSeed;
	temp=(_int16)(MASK&0x00000800)>>10|((MASK&0x00200000)>>21);
	if(temp==0)
	{
		wTemp=(_int32)((wSeed&0xff000000)>>24);
	}
	else if(temp==1)
	{
		wTemp=(_int32)((wSeed&0x00ff0000)>>16);
	}
	else if(temp==2)
	{
		wTemp=(_int32)((wSeed&0x0000ff00)>>8);
	}
	else
	{
		wTemp=(_int32)(wSeed&0x000000ff);
	}

	SB1=(_int32)((MASK&0x000003FC)>>2);
	SB2=(_int32)(((MASK&0x7F800000)>>23^0xA5));
	SB3=(_int32)(((MASK&0x001FE000)>>13^0x5A));
	iterations=(_int32)(((wTemp^SB1)&SB2)+SB3);

	for(jj=0;jj<iterations;jj++)
	{
		wTemp=((wLastSeed&0x40000000)/0x40000000)^
			((wLastSeed&0x01000000)/0x01000000)^
			((wLastSeed&0x1000)/0x1000)^((wLastSeed&0x04)/0x04);

		wLSBit=(wTemp&0x00000001);
		wLastSeed=(_int32)(wLastSeed<<1);
		wTop31Bits=(_int32)(wLastSeed&0xFFFFFFFE);
		wLastSeed=(_int32)(wTop31Bits|wLSBit);
	}
	if(MASK&0x00000001)
	{
		wTop31Bits=((wLastSeed&0x00FF0000)>>16)|((wLastSeed&0xFF000000)>>8)|
					((wLastSeed&0x000000FF)<<8)|((wLastSeed&0x0000FF00)<<16);
	}
	else
	wTop31Bits=wLastSeed;

	wTop31Bits=wTop31Bits^MASK;
	return(wTop31Bits);
}

unsigned long saver::uds_calc_key(unsigned long seed,unsigned char level)
{
	unsigned long temp;

	if(seed == 0)
	{
		seed = NC_DEFAULT_SEED;
	}
	
	if (level == 1) // APP
	{
		SecuriyAlgorithmApp((int*)&seed, (int *)&temp, NC_APP_KEYMASK, 1);

	}
	else if (level == 3) // FBL
	{
		//temp = SecuriyAlgorithmFBL(seed, NC_FBL_KEYMASK);

		CalculateKey_WEICHAI_BootLevel((const unsigned char*)&seed, (unsigned char *)&temp, NC_FBL_KEYMASK);
	}
	else
	{
		temp = 0;
	}

	return temp;
}


//º£Âí 

unsigned long saver::uds_calc_key(unsigned long seed, unsigned char level,unsigned long sN)
{
	unsigned long temp;

	unsigned int i;

	unsigned long v0 = seed;
	unsigned long v1 = ~seed;
	unsigned long sum = 0;
	unsigned long delta = 0x9E3779B9;
	unsigned long k[4] = { 0 };


	if (level == 2) {
		sN = ((sN >> 3) & 0x1F1F1F1F) | ((sN << 5)& 0xE0E0E0E0);
	}
	

	if (level == 3) {
		sN = ((sN >> 3) & 0x1F1F1F1F) | ((sN << 5) & 0xE0E0E0E0);
		sN = ((sN >> 3) & 0x1F1F1F1F) | ((sN << 5) & 0xE0E0E0E0);
	}



	k[0] = (sN >> 24 & 0xFF);
	k[1] = (sN >> 16 & 0xFF);
	k[2] = (sN >> 8 & 0xFF);
	k[3] = (sN >> 0 & 0xFF);

	for (i = 0; i < 2;i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
	
	}
	return v0;
}





/*******************************/