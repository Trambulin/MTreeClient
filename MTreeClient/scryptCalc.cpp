#include<string.h>
#include<chrono>
#include"scryptCalc.h"

std::mutex scryptCalc::variableValuesMtx;

scryptCalc::scryptCalc()
{
	for (size_t i = 0; i < 20; i++) {
		data[i] = 2; target[i] = 0;
	}
	scratchbuf = new unsigned char[1024 * 128 + 63];
	memcpy(midstate, sha256_h, 32);
	sha256_transform(midstate, data, 0);
	V = (uint32_t *)(((uintptr_t)(scratchbuf)+63) & ~(uintptr_t)(63));
}

scryptCalc::scryptCalc(uint32_t datas[20])
{
	memcpy(data, datas, 20 * 4);
	scratchbuf = new unsigned char[1024 * 128 + 63];
	memcpy(midstate, sha256_h, 32);
	sha256_transform(midstate, data, 0);
	V = (uint32_t *)(((uintptr_t)(scratchbuf)+63) & ~(uintptr_t)(63));
}

scryptCalc::~scryptCalc()
{
	delete[] scratchbuf;
}

void scryptCalc::sha256_transform(uint32_t *state, const uint32_t *block, int swap)
{
	uint32_t W[64];
	uint32_t S[8];
	uint32_t t0, t1;
	int i;
	if (swap) {
		for (i = 0; i < 16; i++)
			W[i] = bswap_32(block[i]);
	}
	else
		memcpy(W, block, 64);
	for (i = 16; i < 64; i += 2) {
		W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
		W[i + 1] = s1(W[i - 1]) + W[i - 6] + s0(W[i - 14]) + W[i - 15];
	}
	memcpy(S, state, 32);
	RNDr(S, W, 0); RNDr(S, W, 1); RNDr(S, W, 2); RNDr(S, W, 3); RNDr(S, W, 4); RNDr(S, W, 5); RNDr(S, W, 6); RNDr(S, W, 7);
	RNDr(S, W, 8); RNDr(S, W, 9); RNDr(S, W, 10); RNDr(S, W, 11); RNDr(S, W, 12); RNDr(S, W, 13); RNDr(S, W, 14);
	RNDr(S, W, 15); RNDr(S, W, 16); RNDr(S, W, 17); RNDr(S, W, 18); RNDr(S, W, 19); RNDr(S, W, 20); RNDr(S, W, 21);
	RNDr(S, W, 22); RNDr(S, W, 23); RNDr(S, W, 24); RNDr(S, W, 25); RNDr(S, W, 26); RNDr(S, W, 27); RNDr(S, W, 28);
	RNDr(S, W, 29); RNDr(S, W, 30); RNDr(S, W, 31); RNDr(S, W, 32); RNDr(S, W, 33); RNDr(S, W, 34); RNDr(S, W, 35);
	RNDr(S, W, 36); RNDr(S, W, 37); RNDr(S, W, 38); RNDr(S, W, 39); RNDr(S, W, 40); RNDr(S, W, 41); RNDr(S, W, 42);
	RNDr(S, W, 43); RNDr(S, W, 44); RNDr(S, W, 45); RNDr(S, W, 46); RNDr(S, W, 47); RNDr(S, W, 48); RNDr(S, W, 49);
	RNDr(S, W, 50); RNDr(S, W, 51); RNDr(S, W, 52); RNDr(S, W, 53); RNDr(S, W, 54); RNDr(S, W, 55); RNDr(S, W, 56);
	RNDr(S, W, 57); RNDr(S, W, 58); RNDr(S, W, 59); RNDr(S, W, 60); RNDr(S, W, 61); RNDr(S, W, 62); RNDr(S, W, 63);
	for (i = 0; i < 8; i++)
		state[i] += S[i];
}

void scryptCalc::HMAC_SHA256_80_init(const uint32_t *key, uint32_t *tstate, uint32_t *ostate)
{
	uint32_t ihash[8];
	uint32_t pad[16];
	int i;

	/* tstate is assumed to contain the midstate of key */
	memcpy(pad, key + 16, 16);
	memcpy(pad + 4, keypad, 48);
	sha256_transform(tstate, pad, 0);
	memcpy(ihash, tstate, 32);

	memcpy(ostate, sha256_h, 32);
	for (i = 0; i < 8; i++)
		pad[i] = ihash[i] ^ 0x5c5c5c5c;
	for (; i < 16; i++)
		pad[i] = 0x5c5c5c5c;
	sha256_transform(ostate, pad, 0);

	memcpy(tstate, sha256_h, 32);
	for (i = 0; i < 8; i++)
		pad[i] = ihash[i] ^ 0x36363636;
	for (; i < 16; i++)
		pad[i] = 0x36363636;
	sha256_transform(tstate, pad, 0);
}

void scryptCalc::PBKDF2_SHA256_80_128(const uint32_t *tstate, const uint32_t *ostate, const uint32_t *salt, uint32_t *output)
{
	uint32_t istate[8], ostate2[8];
	uint32_t ibuf[16], obuf[16];
	int i, j;

	memcpy(istate, tstate, 32);
	sha256_transform(istate, salt, 0);

	memcpy(ibuf, salt + 16, 16);
	memcpy(ibuf + 5, innerpad, 44);
	memcpy(obuf + 8, outerpad, 32);

	for (i = 0; i < 4; i++) {
		memcpy(obuf, istate, 32);
		ibuf[4] = i + 1;
		sha256_transform(obuf, ibuf, 0);

		memcpy(ostate2, ostate, 32);
		sha256_transform(ostate2, obuf, 0);
		for (j = 0; j < 8; j++)
			output[8 * i + j] = bswap_32(ostate2[j]);
	}
}

void scryptCalc::xor_salsa8(uint32_t B[16], const uint32_t Bx[16])
{
	uint32_t x00, x01, x02, x03, x04, x05, x06, x07, x08, x09, x10, x11, x12, x13, x14, x15;
	int i;
	x00 = (B[0] ^= Bx[0]); x01 = (B[1] ^= Bx[1]); x02 = (B[2] ^= Bx[2]); x03 = (B[3] ^= Bx[3]);
	x04 = (B[4] ^= Bx[4]); x05 = (B[5] ^= Bx[5]); x06 = (B[6] ^= Bx[6]); x07 = (B[7] ^= Bx[7]);
	x08 = (B[8] ^= Bx[8]); x09 = (B[9] ^= Bx[9]); x10 = (B[10] ^= Bx[10]); x11 = (B[11] ^= Bx[11]);
	x12 = (B[12] ^= Bx[12]); x13 = (B[13] ^= Bx[13]); x14 = (B[14] ^= Bx[14]); x15 = (B[15] ^= Bx[15]);

	for (i = 0; i < 8; i += 2) {
#define R(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
		/* Operate on columns. */
		x04 ^= R(x00 + x12, 7); x09 ^= R(x05 + x01, 7);
		x14 ^= R(x10 + x06, 7); x03 ^= R(x15 + x11, 7);
		x08 ^= R(x04 + x00, 9); x13 ^= R(x09 + x05, 9);
		x02 ^= R(x14 + x10, 9); x07 ^= R(x03 + x15, 9);
		x12 ^= R(x08 + x04, 13); x01 ^= R(x13 + x09, 13);
		x06 ^= R(x02 + x14, 13); x11 ^= R(x07 + x03, 13);
		x00 ^= R(x12 + x08, 18); x05 ^= R(x01 + x13, 18);
		x10 ^= R(x06 + x02, 18); x15 ^= R(x11 + x07, 18);
		/* Operate on rows. */
		x01 ^= R(x00 + x03, 7); x06 ^= R(x05 + x04, 7);
		x11 ^= R(x10 + x09, 7); x12 ^= R(x15 + x14, 7);
		x02 ^= R(x01 + x00, 9); x07 ^= R(x06 + x05, 9);
		x08 ^= R(x11 + x10, 9); x13 ^= R(x12 + x15, 9);
		x03 ^= R(x02 + x01, 13); x04 ^= R(x07 + x06, 13);
		x09 ^= R(x08 + x11, 13); x14 ^= R(x13 + x12, 13);
		x00 ^= R(x03 + x02, 18); x05 ^= R(x04 + x07, 18);
		x10 ^= R(x09 + x08, 18); x15 ^= R(x14 + x13, 18);
#undef R
	}
	B[0] += x00; B[1] += x01; B[2] += x02; B[3] += x03; B[4] += x04; B[5] += x05; B[6] += x06; B[7] += x07;
	B[8] += x08; B[9] += x09; B[10] += x10; B[11] += x11; B[12] += x12; B[13] += x13; B[14] += x14; B[15] += x15;
}

void scryptCalc::scrypt_core(uint32_t *X, uint32_t *V, int N)
{
	int i;

	for (i = 0; i < N; i++) {
		memcpy(&V[i * 32], X, 128);
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}
	for (i = 0; i < N; i++) {
		uint32_t j = 32 * (X[16] & (N - 1));
		for (uint8_t k = 0; k < 32; k++)
			X[k] ^= V[j + k];
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}
}

void scryptCalc::PBKDF2_SHA256_128_32(uint32_t *tstate, uint32_t *ostate, const uint32_t *salt, uint32_t *output)
{
	uint32_t buf[16];
	int i;

	sha256_transform(tstate, salt, 1);
	sha256_transform(tstate, salt + 16, 1);
	sha256_transform(tstate, finalblk, 0);
	memcpy(buf, tstate, 32);
	memcpy(buf + 8, outerpad, 32);

	sha256_transform(ostate, buf, 0);
	for (i = 0; i < 8; i++)
		output[i] = bswap_32(ostate[i]);
}

bool scryptCalc::hashTargetTest()
{
	for (int i = 7; i >= 0; i--) {
		if (hash[i] > target[i]) {
			return false;
		}
		if (hash[i] < target[i]) {
			return true;
		}
	}
}

void scryptCalc::runScrypt(uint32_t beginValue, uint32_t lastValue)
{
	int cycleCounter = 0;
	std::chrono::time_point<std::chrono::steady_clock> endT;
	std::chrono::time_point<std::chrono::steady_clock> startT = std::chrono::high_resolution_clock::now();
	do {
		cycleCounter++;
		data[19] = beginValue++;
		uint32_t tstate[8], ostate[8];
		uint32_t X[32];
		uint32_t *V;
		V = (uint32_t *)(((uintptr_t)(scratchbuf)+63) & ~(uintptr_t)(63));
		memcpy(tstate, midstate, 32);
		HMAC_SHA256_80_init(data, tstate, ostate);
		PBKDF2_SHA256_80_128(tstate, ostate, data, X);
		scrypt_core(X, V, 1024);
		PBKDF2_SHA256_128_32(tstate, ostate, X, hash);
		if (hash[7] <= target[7] && hashTargetTest()) {
			//SUBMIT
		}
		if (beginValue%500==1 && calcOver) { //threadOver check is slow, enough to check every ~(half a sec)
				break;
		}
		if (cycleCounter > 1000) {
			endT = std::chrono::high_resolution_clock::now();
			cycleTime = std::chrono::duration_cast<std::chrono::milliseconds>(endT - startT).count();
			startT = std::chrono::high_resolution_clock::now();
			cycleCounter = 0;
		}
	} while (beginValue < lastValue);
}

void scryptCalc::runAlgorithm()
{
	runScrypt(firstVal, lastVal);
}