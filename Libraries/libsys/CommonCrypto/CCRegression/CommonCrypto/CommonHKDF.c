//
//  CommonHKDF.c
//  CommonCrypto
//
//	Copyright 2014 Apple Inc. All rights reserved.
//

#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"

#if (CCHKDFTEST == 0)
entryPoint(CommonHKDF,"CommonHKDF test")
#else
#include <CommonCrypto/CommonKeyDerivationSPI.h>

#define type_sha1		1
#define type_sha256		256
#define type_sha512		512

typedef struct {
	int					type;
	const char *		ikm;
	const char *		salt;
	const char *		info;
	size_t				len;
	const char *		okm;
} test_vector_t;

static const test_vector_t	hkdf_sha256_tests[] = {
	// RFC 5869 Test Case 1
	{
	/* Type */	type_sha256, 
	/* IKM */	"0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", 
	/* Salt */	"000102030405060708090a0b0c", 
	/* Info */	"f0f1f2f3f4f5f6f7f8f9", 
	/* Len */	42, 
	/* OKM */	"3cb25f25faacd57a90434f64d0362f2a"
				"2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
				"34007208d5b887185865"
	}, 
	// RFC 5869 Test Case 2
	{
	/* Type */	type_sha256, 
	/* IKM */	"000102030405060708090a0b0c0d0e0f"
				"101112131415161718191a1b1c1d1e1f"
				"202122232425262728292a2b2c2d2e2f"
				"303132333435363738393a3b3c3d3e3f"
				"404142434445464748494a4b4c4d4e4f", 
	/* Salt */	"606162636465666768696a6b6c6d6e6f"
				"707172737475767778797a7b7c7d7e7f"
				"808182838485868788898a8b8c8d8e8f"
				"909192939495969798999a9b9c9d9e9f"
				"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf", 
	/* Info */	"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
				"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
				"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
				"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
				"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff", 
	/* Len */	82, 
	/* OKM */	"b11e398dc80327a1c8e7f78c596a4934"
				"4f012eda2d4efad8a050cc4c19afa97c"
				"59045a99cac7827271cb41c65e590e09"
				"da3275600c2f09b8367793a9aca3db71"
				"cc30c58179ec3e87c14c01d5c1f3434f"
				"1d87"
	},
	// RFC 5869 Test Case 3
	{
	/* Type */	type_sha256, 
	/* IKM */	"0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", 
	/* Salt */	"", 
	/* Info */	"", 
	/* Len */	42, 
	/* OKM */	"8da4e775a563c18f715f802a063c5a31"
				"b8a11f5c5ee1879ec3454e5f3c738d2d"
				"9d201395faa4b61a96c8"
	},
	// RFC 5869 Test Case 4
	{
	/* Type */	type_sha1, 
	/* IKM */	"0b0b0b0b0b0b0b0b0b0b0b", 
	/* Salt */	"000102030405060708090a0b0c", 
	/* Info */	"f0f1f2f3f4f5f6f7f8f9", 
	/* Len */	42, 
	/* OKM */	"085a01ea1b10f36933068b56efa5ad81"
				"a4f14b822f5b091568a9cdd4f155fda2"
				"c22e422478d305f3f896"
	},
	// RFC 5869 Test Case 5
	{
	/* Type */	type_sha1, 
	/* IKM */	"000102030405060708090a0b0c0d0e0f"
				"101112131415161718191a1b1c1d1e1f"
				"202122232425262728292a2b2c2d2e2f"
				"303132333435363738393a3b3c3d3e3f"
				"404142434445464748494a4b4c4d4e4f", 
	/* Salt */	"606162636465666768696a6b6c6d6e6f"
				"707172737475767778797a7b7c7d7e7f"
				"808182838485868788898a8b8c8d8e8f"
				"909192939495969798999a9b9c9d9e9f"
				"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf", 
	/* Info */	"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
				"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
				"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
				"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
				"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff", 
	/* Len */	82, 
	/* OKM */	"0bd770a74d1160f7c9f12cd5912a06eb"
				"ff6adcae899d92191fe4305673ba2ffe"
				"8fa3f1a4e5ad79f3f334b3b202b2173c"
				"486ea37ce3d397ed034c7f9dfeb15c5e"
				"927336d0441f4c4300e2cff0d0900b52"
				"d3b4"
	},
	// RFC 5869 Test Case 6
	{
	/* Type */	type_sha1, 
	/* IKM */	"0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", 
	/* Salt */	"", 
	/* Info */	"", 
	/* Len */	42, 
	/* OKM */	"0ac1af7002b3d761d1e55298da9d0506"
				"b9ae52057220a306e07b6b87e8df21d0"
				"ea00033de03984d34918"
	},
	// RFC 5869 Test Case 7
	{
	/* Type */	type_sha1, 
	/* IKM */	"0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c", 
	/* Salt */	"", 
	/* Info */	"", 
	/* Len */	42, 
	/* OKM */	"2c91117204d745f3500d636a62f64f0a"
				"b3bae548aa53d423b0d1f27ebba6f5e5"
				"673a081d70cce7acfc48"
	},
	// RFC 5869 Test Case 1 (updated for SHA-512)
	{
	/* Type */	type_sha512, 
	/* IKM */	"0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B", 
	/* Salt */	"000102030405060708090A0B0C", 
	/* Info */	"F0F1F2F3F4F5F6F7F8F9", 
	/* Len */	42, 
	/* OKM */	"832390086CDA71FB47625BB5CEB168E4"
				"C8E26A1A16ED34D9FC7FE92C14815793"
				"38DA362CB8D9F925D7CB"
	},
	// RFC 5869 Test Case 2 (updated for SHA-512)
	{
	/* Type */	type_sha512, 
	/* IKM */	"000102030405060708090A0B0C0D0E0F"
				"101112131415161718191A1B1C1D1E1F"
				"202122232425262728292A2B2C2D2E2F"
				"303132333435363738393A3B3C3D3E3F"
				"404142434445464748494A4B4C4D4E4F", 
	/* Salt */	"606162636465666768696A6B6C6D6E6F"
				"707172737475767778797A7B7C7D7E7F"
				"808182838485868788898A8B8C8D8E8F"
				"909192939495969798999A9B9C9D9E9F"
				"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF", 
	/* Info */	"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
				"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
				"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
				"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
				"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF", 
	/* Len */	82, 
	/* OKM */	"CE6C97192805B346E6161E821ED16567"
				"3B84F400A2B514B2FE23D84CD189DDF1"
				"B695B48CBD1C8388441137B3CE28F16A"
				"A64BA33BA466B24DF6CFCB021ECFF235"
				"F6A2056CE3AF1DE44D572097A8505D9E"
				"7A93"
	},
	// RFC 5869 Test Case 3 (updated for SHA-512)
	{
	/* Type */	type_sha512, 
	/* IKM */	"0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B", 
	/* Salt */	"", 
	/* Info */	"", 
	/* Len */	42, 
	/* OKM */	"F5FA02B18298A72A8C23898A8703472C"
				"6EB179DC204C03425C970E3B164BF90F"
				"FF22D04836D0E2343BAC"
	},
    // No key
    {
        /* Type */    type_sha512,
        /* IKM */     "",
        /* Salt */    "",
        /* Info */    "",
        /* Len */    42,
        /* OKM */    "9d73c98e791e80ebe5b4cb45693aa32fdd44b5fa3edab3ec82f9d0f4d66905e2215ad0d4ac20fe570da5"
    },
};	

int CommonHKDF(int __unused argc, char *const * __unused argv)
{
	size_t i, n;
	int err;
	
	plan_tests(11 * 3);
		
	n = sizeof(hkdf_sha256_tests) / sizeof(*hkdf_sha256_tests);
	for(i = 0; i < n; ++i) {
		const test_vector_t *			tv   = &hkdf_sha256_tests[ i ];
		byteBuffer						ikm  = hexStringToBytes(tv->ikm);
		byteBuffer						salt = hexStringToBytes(tv->salt);
		byteBuffer						info = hexStringToBytes(tv->info);
		byteBuffer						okmActual = mallocByteBuffer(tv->len);
		byteBuffer						okmExpected = hexStringToBytes(tv->okm);
		CCDigestAlgorithm				digestType;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		if(     tv->type == type_sha1)   digestType = kCCDigestSHA1;
#pragma clang diagnostic pop
		else if(tv->type == type_sha256) digestType = kCCDigestSHA256;
		else if(tv->type == type_sha512) digestType = kCCDigestSHA512;
		else abort();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		err = CCKeyDerivationHMac(kCCKDFAlgorithmHKDF, digestType, 0, ikm->bytes, ikm->len, NULL, 0, 
			info->bytes, info->len, NULL, 0, salt->bytes, salt->len, okmActual->bytes, okmActual->len );
#pragma clang diagnostic pop
		ok(!err, "check return value");
        is(okmActual->len, okmExpected->len, "compare length");
		ok_memcmp(okmActual->bytes, okmExpected->bytes, okmExpected->len, "compare memory of answer");
		
		free(ikm);
		free(salt);
		free(info);
		free(okmActual);
		free(okmExpected);
	}
	return 0;
}
#endif // CCHKDFTEST
