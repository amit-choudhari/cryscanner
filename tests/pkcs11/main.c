#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include "cryptoki.h"
#include "pkcs11.h"

typedef unsigned char   uchar_t;
typedef unsigned short  ushort_t;
typedef unsigned int    uint_t;
typedef unsigned long   ulong_t;

/* Key template related definitions. */
static CK_BBOOL truevalue = TRUE;
static CK_BBOOL falsevalue = FALSE;
static CK_OBJECT_CLASS class = CKO_SECRET_KEY;
static CK_KEY_TYPE keyType = CKK_DES;
// CKA_TOKEN
const CK_BBOOL ON_TOKEN = CK_TRUE;
const CK_BBOOL IN_SESSION = CK_FALSE;

// CKA_PRIVATE
const CK_BBOOL IS_PRIVATE = CK_TRUE;
const CK_BBOOL IS_PUBLIC = CK_FALSE;

#define NR_OF_BLOCKS_IN_TEST 0x10001

CK_RV generateGenericKey(CK_SESSION_HANDLE hSession, CK_BBOOL bToken, CK_BBOOL bPrivate, CK_OBJECT_HANDLE *hKey)
{
        CK_MECHANISM mechanism = { CKM_GENERIC_SECRET_KEY_GEN, NULL_PTR, 0 };
        CK_ULONG bytes = 16;
        // CK_BBOOL bFalse = CK_FALSE;
        CK_BBOOL bTrue = CK_TRUE;
        CK_ATTRIBUTE keyAttribs[] = {
                { CKA_TOKEN, &bToken, sizeof(bToken) },
                { CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
                { CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
                { CKA_DECRYPT, &bTrue, sizeof(bTrue) },
                { CKA_WRAP, &bTrue, sizeof(bTrue) },
                { CKA_UNWRAP, &bTrue, sizeof(bTrue) },
                { CKA_VALUE_LEN, &bytes, sizeof(bytes) },
        };

        *hKey = CK_INVALID_HANDLE;
	CK_RV rv = C_GenerateKey(hSession, &mechanism,
                             keyAttribs, sizeof(keyAttribs)/sizeof(CK_ATTRIBUTE),
                             hKey);
	return rv;
}

CK_RV generateAesKey(CK_SESSION_HANDLE hSession, CK_BBOOL bToken, CK_BBOOL bPrivate, CK_OBJECT_HANDLE *hKey)
{
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_ULONG bytes = 16;
	// CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_TOKEN, &bToken, sizeof(bToken) },
		{ CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_WRAP, &bTrue, sizeof(bTrue) },
		{ CKA_UNWRAP, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &bytes, sizeof(bytes) },
	};

	*hKey = CK_INVALID_HANDLE;
	return C_GenerateKey(hSession, &mechanism,
			     keyAttribs, sizeof(keyAttribs)/sizeof(CK_ATTRIBUTE),
			     hKey);
}

void encryptDecrypt(
		const CK_MECHANISM_TYPE mechanismType,
		const size_t blockSize,
		const CK_SESSION_HANDLE hSession,
		const CK_OBJECT_HANDLE hKey,
		const size_t messageSize,
		const int isSizeOK)
{
	char vData[32] = {'A','M','I','T','\0'};
	char vEncryptedData[100] = {0};

	CK_RV rv;
/*
	rv = C_GenerateRandom(hSession, (CK_BYTE_PTR)vData, sizeof(vData));
	if (rv != CKR_OK)
		goto exit;
*/

	const CK_MECHANISM mechanism = { mechanismType, NULL_PTR, 0 };
	CK_MECHANISM_PTR pMechanism = (CK_MECHANISM_PTR)&mechanism;
	CK_AES_CTR_PARAMS ctrParams =
	{
		32,
		{
			0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
		}
	};
	CK_BYTE gcmIV[] = {
		0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE,
		0xDB, 0xAD, 0xDE, 0xCA, 0xF8, 0x88
	};
	CK_BYTE gcmAAD[] = {
		0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
		0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
		0xAB, 0xAD, 0xDA, 0xD2
	};
	CK_GCM_PARAMS gcmParams =
	{
		&gcmIV[0],
		sizeof(gcmIV),
		sizeof(gcmIV)*8,
		&gcmAAD[0],
		sizeof(gcmAAD),
		16*8
	};

	switch (mechanismType)
	{
		case CKM_DES_CBC:
		case CKM_DES_CBC_PAD:
		case CKM_DES3_CBC:
		case CKM_DES3_CBC_PAD:
		case CKM_AES_CBC:
		case CKM_AES_CBC_PAD:
			pMechanism->pParameter = (CK_VOID_PTR)vData;
			pMechanism->ulParameterLen = blockSize;
			break;
		case CKM_AES_CTR:
			pMechanism->pParameter = &ctrParams;
			pMechanism->ulParameterLen = sizeof(ctrParams);
			break;
		case CKM_AES_GCM:
			pMechanism->pParameter = &gcmParams;
			pMechanism->ulParameterLen = sizeof(gcmParams);
			break;
		default:
			break;
	}

	// Single-part encryption
	rv = C_EncryptInit(hSession,pMechanism,hKey);
	if (rv != CKR_OK)
		goto exit;
	CK_ULONG ulEncryptedDataLen;
	{
		rv = C_Encrypt(hSession,(CK_BYTE_PTR)vData,sizeof(vData),NULL_PTR,&ulEncryptedDataLen);
		if (rv != CKR_OK)
			goto exit;
		if ( isSizeOK ) {
			rv = C_Encrypt(hSession,(CK_BYTE_PTR)vData,sizeof(vData),vEncryptedData,&ulEncryptedDataLen);
			if (rv != CKR_OK)
				goto exit;
		} else {
			fprintf(stderr,"C_Encrypt should fail with C_CKR_DATA_LEN_RANGE");
		}
	}


	// Single-part decryption
	rv = C_DecryptInit(hSession,pMechanism,hKey);
	if (rv != CKR_OK)
		goto exit;

	{
		CK_ULONG ulDataLen;
		rv = C_Decrypt(hSession,vEncryptedData,ulEncryptedDataLen,NULL_PTR,&ulDataLen);
		if (rv != CKR_OK)
			goto exit;
		if ( isSizeOK ) {
			CK_BYTE vDecryptedData[100] = {0};
			rv = C_Decrypt(hSession,vEncryptedData,ulEncryptedDataLen,vDecryptedData,&ulDataLen);
			if (rv != CKR_OK)
				goto exit;
			fprintf(stderr,"C_Encrypt C_Decrypt does not give the original %s == %s", vData,vDecryptedData);
		} else {
			fprintf(stderr, "C_Decrypt should fail with CKR_ENCRYPTED_DATA_LEN_RANGE");
		}
	}
exit:
	if (rv != CKR_OK) {
		fprintf(stderr, "SUNW_C_GetMechSession: rv = 0x%.8X\n", rv);
		exit(1);
	}

}

int main()
{
	CK_RV rv;
        CK_SESSION_HANDLE hSession;
        CK_SESSION_HANDLE hSessionRO;
	CK_SLOT_ID slotID;
	CK_UTF8CHAR label[32];
	CK_ULONG m_soPin1Length;
	CK_UTF8CHAR_PTR m_soPin1;
	CK_UTF8CHAR_PTR m_userPin1;
	CK_ULONG m_userPin1Length;

        // Just make sure that we finalize any previous tests
       	rv = C_Finalize(NULL_PTR);
	if (rv != CKR_OK) {
		fprintf(stderr, "not initialized\n");
	}

        // Initialize the library and start the test.
        rv = C_Initialize(NULL_PTR);
	if (rv != CKR_OK) {
		fprintf(stderr, "Login failed\n");
		goto exit;
	}

	slotID = 610184248;
	m_soPin1 = (CK_UTF8CHAR_PTR)"1234";
	m_soPin1Length = strlen((char*)m_soPin1);
	m_userPin1 = (CK_UTF8CHAR_PTR)"1234";
	m_userPin1Length = strlen((char*)m_userPin1);
	memset(label, ' ', 32);
	memcpy(label, "first", strlen("first"));
        // Open read-write session
        rv = C_OpenSession(slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL_PTR, NULL_PTR, &hSession);
	if (rv != CKR_OK) {
		fprintf(stderr, "open session failed\n");
		goto exit;
	}
	rv = C_OpenSession(slotID, CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &hSessionRO);
	if (rv != CKR_OK) {
		fprintf(stderr, "open session failed\n");
		goto exit;
	}

        // Login USER into the session so we can create a private objects
        rv = C_Login(hSessionRO,CKU_USER,m_userPin1,m_userPin1Length);
	if (rv != CKR_OK) {
		fprintf(stderr, "Login failed\n");
		goto exit;
	}

        CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;

        // Generate a session key.
        /*rv = generateGenericKey(hSession,IN_SESSION,IS_PUBLIC,&hKey);
	if (rv != CKR_OK) {
		fprintf(stderr, "generic key failed\n");
		goto exit;
	}*/

        hKey = CK_INVALID_HANDLE;
	rv = generateAesKey(hSession,IN_SESSION,IS_PUBLIC,&hKey);
	if (rv != CKR_OK) {
		fprintf(stderr, "aes gen failed\n");
		goto exit;
	}

	int blockSize = 0x10;
	encryptDecrypt(CKM_AES_ECB,blockSize,hSessionRO,hKey,blockSize*NR_OF_BLOCKS_IN_TEST-1,1);
	fprintf(stderr, "completed\n");

exit:
	if (rv != CKR_OK) {
		fprintf(stderr, "SUNW_C_GetMechSession: rv = 0x%.8X\n", rv);
		exit(1);
	}
}
