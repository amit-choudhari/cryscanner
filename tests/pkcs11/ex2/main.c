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
#define CHECK_RV \
		if (rv != CKR_OK) { \
			fprintf(stderr, "%s:%d\n",__func__, __LINE__);\
			goto exit;\
		}


#define ASSERT_KEY_EXTRACTABLE(hKey) { \
        	CK_BBOOL bCheck; \
	      	CK_ATTRIBUTE checkAttribs[] = { { CKA_EXTRACTABLE, &bCheck, sizeof(bCheck) } }; \
		rv = C_GetAttributeValue(hSession, *hDerive, checkAttribs, sizeof(checkAttribs)/sizeof(CK_ATTRIBUTE)); \
		CHECK_RV;\
		fprintf(stderr, "is Extractable: %d\n",*(CK_BBOOL*)checkAttribs[0].pValue);\
		}

#define ASSERT_KEY_SENSITIVE(hKey) { \
        	CK_BBOOL bCheck; \
	      	CK_ATTRIBUTE checkAttribs[] = { { CKA_SENSITIVE, &bCheck, sizeof(bCheck) } }; \
		rv = C_GetAttributeValue(hSession, *hDerive, checkAttribs, sizeof(checkAttribs)/sizeof(CK_ATTRIBUTE)); \
		CHECK_RV;\
		fprintf(stderr, "is Sensitive: %d\n",*(CK_BBOOL*)checkAttribs[0].pValue);\
		}

#define ASSERT_KEY_WRAP(hKey) { \
        	CK_BBOOL bCheck; \
	      	CK_ATTRIBUTE checkAttribs[] = { { CKA_WRAP, &bCheck, sizeof(bCheck) } }; \
		rv = C_GetAttributeValue(hSession, *hDerive, checkAttribs, sizeof(checkAttribs)/sizeof(CK_ATTRIBUTE)); \
		CHECK_RV;\
		fprintf(stderr, "is Wrap: %d\n",*(CK_BBOOL*)checkAttribs[0].pValue);\
		}

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

CK_RV generateDes2Key(CK_SESSION_HANDLE hSession, CK_BBOOL bToken, CK_BBOOL bPrivate, CK_OBJECT_HANDLE *hKey)
{
	CK_MECHANISM mechanism = { CKM_DES2_KEY_GEN, NULL_PTR, 0 };
	CK_ULONG bytes = 16;
	// CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_TOKEN, &bToken, sizeof(bToken) },
		{ CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
		{ CKA_SENSITIVE, &bTrue, sizeof(bTrue) },
		{ CKA_DERIVE, &bTrue, sizeof(bTrue) }
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


void symDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE *hDerive, CK_MECHANISM_TYPE mechType, CK_KEY_TYPE keyType)
{
	CK_RV rv;
	CK_KEY_TYPE keyType2;
	CK_MECHANISM mechanism = { mechType, NULL_PTR, 0 };
	CK_KEY_DERIVATION_STRING_DATA derivation_data = { NULL_PTR, 0L } ;
	//CK_MECHANISM mechanism = { CKM_XOR_BASE_AND_DATA, &derivation_data, sizeof(derivation_data)};
	CK_MECHANISM mechEncrypt = { CKM_VENDOR_DEFINED, NULL_PTR, 0 };
	CK_KEY_DERIVATION_STRING_DATA param1;
	CK_DES_CBC_ENCRYPT_DATA_PARAMS param2;
	CK_AES_CBC_ENCRYPT_DATA_PARAMS param3;

	CK_BYTE data[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
		0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
		0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32
	};
	CK_ULONG secLen = 0;

	switch (mechType)
	{
		case CKM_DES_ECB_ENCRYPT_DATA:
		case CKM_DES3_ECB_ENCRYPT_DATA:
		case CKM_AES_ECB_ENCRYPT_DATA:
			param1.pData = &data[0];
			param1.ulLen = sizeof(data);
			mechanism.pParameter = &param1;
			mechanism.ulParameterLen = sizeof(param1);
			break;
		case CKM_DES_CBC_ENCRYPT_DATA:
		case CKM_DES3_CBC_ENCRYPT_DATA:
			memcpy(param2.iv, "12345678", 8);
			param2.pData = &data[0];
			param2.length = sizeof(data);
			mechanism.pParameter = &param2;
			mechanism.ulParameterLen = sizeof(param2);
			break;
		case CKM_AES_CBC_ENCRYPT_DATA:
			memcpy(param3.iv, "1234567890ABCDEF", 16);
			param3.pData = &data[0];
			param3.length = sizeof(data);
			mechanism.pParameter = &param3;
			mechanism.ulParameterLen = sizeof(param3);
			break;
		default:
			fprintf(stderr,"Invalid mechanism");
	}

	switch (keyType)
	{
		case CKK_GENERIC_SECRET:
			secLen = 32;
			break;
		case CKK_DES:
			mechEncrypt.mechanism = CKM_DES_ECB;
			break;
		case CKK_DES2:
		case CKK_DES3:
			mechEncrypt.mechanism = CKM_DES3_ECB;
			break;
		case CKK_AES:
			mechEncrypt.mechanism = CKM_AES_ECB;
			secLen = 32;
			break;
		default:
			fprintf(stderr,"Invalid key type");
	}

	CK_OBJECT_CLASS keyClass = CKO_SECRET_KEY;
	CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_CLASS, &keyClass, sizeof(keyClass) },
		{ CKA_KEY_TYPE, &keyType, sizeof(keyType) },
                { CKA_WRAP, &bTrue, sizeof(bTrue) },
		{ CKA_PRIVATE, &bFalse, sizeof(bFalse) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bTrue, sizeof(bFalse) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &secLen, sizeof(secLen) }
	};

	*hDerive = CK_INVALID_HANDLE;
	if (secLen > 0)
	{
		rv = C_DeriveKey(hSession, &mechanism, hKey,
				 keyAttribs, sizeof(keyAttribs)/sizeof(CK_ATTRIBUTE),
				 hDerive);
	}
	else
	{
		rv = C_DeriveKey(hSession, &mechanism, hKey,
				 keyAttribs, sizeof(keyAttribs)/sizeof(CK_ATTRIBUTE) - 1,
				 hDerive);
	}
	if (rv != CKR_OK) {
		fprintf(stderr, "%s:%d\n",__func__, __LINE__);
		goto exit;
	}

	// Check that KCV has been set
	CK_ATTRIBUTE checkAttribs[] = {
		{ CKA_CHECK_VALUE, NULL_PTR, 0 },
            	{ CKA_KEY_TYPE, &keyType2, sizeof(keyType2) }
	};
	CK_BYTE check[3];
	checkAttribs[0].pValue = check;
	checkAttribs[0].ulValueLen = sizeof(check);
	fprintf(stderr, "%s:%d %x\n",__func__, __LINE__, keyType2);
	rv = C_GetAttributeValue(hSession, *hDerive, checkAttribs, 2);
	if (rv != CKR_OK) {
		fprintf(stderr, "%s:%d\n",__func__, __LINE__);
		goto exit;
	}
	if(checkAttribs[0].ulValueLen != 3){
		fprintf(stderr, "%s:%d\n",__func__, __LINE__);
		goto exit;
	}
	ASSERT_KEY_EXTRACTABLE(hKey)
	ASSERT_KEY_SENSITIVE(hKey)
	ASSERT_KEY_WRAP(hKey)

	fprintf(stderr, "%s:%d %x\n",__func__, __LINE__, keyType2);

	if (keyType == CKK_GENERIC_SECRET) return;

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
        CK_OBJECT_HANDLE hKeyDes2 = CK_INVALID_HANDLE;
	//rv = generateAesKey(hSession,IN_SESSION,IS_PUBLIC,&hKey);
	//rv = generateDesKey(hSession,IN_SESSION,IS_PUBLIC,&hKey);
        rv = generateDes2Key(hSession,IN_SESSION,IS_PUBLIC,&hKeyDes2);
	if (rv != CKR_OK) {
		fprintf(stderr, "aes gen failed\n");
		goto exit;
	}

	CK_OBJECT_HANDLE hDerive = CK_INVALID_HANDLE;
	//symDerive(hSession,hKey,&hDerive,CKM_DES_ECB_ENCRYPT_DATA,CKK_DES);
	symDerive(hSession,hKeyDes2,&hDerive,CKM_DES3_ECB_ENCRYPT_DATA,CKK_DES2);

	int blockSize = 0x10;
	//encryptDecrypt(CKM_AES_ECB,blockSize,hSessionRO,hKey,blockSize*NR_OF_BLOCKS_IN_TEST-1,1);
	fprintf(stderr, "completed\n");

exit:
	if (rv != CKR_OK) {
		fprintf(stderr, "SUNW_C_GetMechSession: rv = 0x%.8X\n", rv);
		exit(1);
	}
}
