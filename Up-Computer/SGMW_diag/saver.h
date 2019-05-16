#pragma once

//typedef unsigned long 

class saver
{
public:
	saver(void);
	~saver(void);
	unsigned long		nc_uds_keymask;

	void SecuriyAlgorithmApp(int *seed, int *key, int MASK, int len);
	void CalculateKey_WEICHAI_BootLevel(const unsigned char* ipSeedArray, unsigned char*iopKeyArray, _int32 MASK);
	_int32 SecuriyAlgorithmFBL(_int32 wSeed,_int32 MASK);

	unsigned long		uds_calc_key(unsigned long seed, unsigned char level);
	unsigned long uds_calc_key(unsigned long seed, unsigned char level, unsigned long sN);
};

