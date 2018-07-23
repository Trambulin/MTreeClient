#pragma once
#include<stdint.h>
#include"algCalc.h"

#define Ch(x, y, z)     ((x & (y ^ z)) ^ z)
#define Maj(x, y, z)    ((x & (y | z)) | (y & z))
#define ROTR(x, n)      ((x >> n) | (x << (32 - n)))
#define S0(x)           (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)           (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)           (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
#define s1(x)           (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))
#define R(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

#define RND(a, b, c, d, e, f, g, h, k) \
	do { \
		t0 = h + S1(e) + Ch(e, f, g) + k; \
		t1 = S0(a) + Maj(a, b, c); \
		d += t0; \
		h  = t0 + t1; \
	} while (0)

#define bswap_32(x) ((((x) << 24) & 0xff000000u) | (((x) << 8) & 0x00ff0000u) \
                   | (((x) >> 8) & 0x0000ff00u) | (((x) >> 24) & 0x000000ffu))

const uint32_t keypad[12] = {
	0x80000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x00000280
};
const uint32_t innerpad[11] = {
	0x80000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x000004a0
};
const uint32_t outerpad[8] = {
	0x80000000, 0, 0, 0, 0, 0, 0, 0x00000300
};
const uint32_t finalblk[16] = {
	0x00000001, 0x80000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x00000620
};
const uint32_t sha256_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
const uint32_t sha256_h[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#define RNDr(S, W, i) \
	RND(S[(64 - i) % 8], S[(65 - i) % 8], \
	    S[(66 - i) % 8], S[(67 - i) % 8], \
	    S[(68 - i) % 8], S[(69 - i) % 8], \
	    S[(70 - i) % 8], S[(71 - i) % 8], \
	    W[i] + sha256_k[i])

class scryptCalc : public algCalc
{
private:
	uint32_t midstate[8];
	uint32_t *V;
	unsigned char *scratchbuf;

	void sha256_transform(uint32_t *state, const uint32_t *block, int swap);
	void HMAC_SHA256_80_init(const uint32_t *key, uint32_t *tstate, uint32_t *ostate);
	void PBKDF2_SHA256_80_128(const uint32_t *tstate, const uint32_t *ostate, const uint32_t *salt, uint32_t *output);
	void xor_salsa8(uint32_t B[16], const uint32_t Bx[16]);
	void scrypt_core(uint32_t *X, uint32_t *V, int N);
	void PBKDF2_SHA256_128_32(uint32_t *tstate, uint32_t *ostate, const uint32_t *salt, uint32_t *output);
	bool hashTargetTest();
public:
	scryptCalc();
	scryptCalc(uint32_t datas[20]);
	~scryptCalc();

	uint32_t hash[8], target[8]; //output hash to check (dont need to be public?)
	uint32_t data[20]; //input datas
	uint32_t firstVal, lastVal;
	void runScrypt(uint32_t beginValue, uint32_t lastValue);
	void runAlgorithm();

	static std::mutex variableValuesMtx;
};