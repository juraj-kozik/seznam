#include "EncryptionClass.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include <sal.h> 
#include <string>
#include <vector>
#define _WINSOCKAPI_ 


#define KEYLENGTH 0x04000000
#define ENCRYPT_ALGORITHM CALG_AES_128
#define ENCRYPT_PROV MS_ENH_RSA_AES_PROV

EncryptionClass::EncryptionClass()
{
	exchanged = false;
}


EncryptionClass::~EncryptionClass()
{
	if (exchanged)
	{
		CryptDestroyKey(hPubKey);
		CryptReleaseContext(hProv, 0);
	}
}

bool EncryptionClass::setup(void * Port)
{
	if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, dwFlags))
	{
		return false;
	}

	if (!CryptGenKey(hProv, CALG_RSA_KEYX, KEYLENGTH | CRYPT_EXPORTABLE, &hPubKey))
	{
		return false;
	}

	if (!CryptGenKey(hProv, CALG_AES_128, CRYPT_EXPORTABLE, &AESKey))
	{
		return false;
	}

	return true;
}

bool EncryptionClass::exchange1(void * Port)
{
	DWORD dwPublicKeyBlobLen, dwAesKeyBlobLen;
	BYTE * pbPublicBlob, *AesBlob, *pbUpperBuffer;

	if (!CryptExportKey(hPubKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwPublicKeyBlobLen))
	{
		return false;
	}

	pbPublicBlob = new BYTE[dwPublicKeyBlobLen];
	pbUpperBuffer = new BYTE[dwPublicKeyBlobLen];

	if (!CryptExportKey(hPubKey, NULL, PUBLICKEYBLOB, 0, pbPublicBlob, &dwPublicKeyBlobLen))
	{
		return false;
	}

	DWORD bytesWritten;
	for (unsigned int i = 0; i < dwPublicKeyBlobLen; ++i)
	{
		if (pbPublicBlob[i] & 0x80)
			pbUpperBuffer[i] = 0x08;
	}

	if (!WriteFile(Port, pbPublicBlob, dwPublicKeyBlobLen, &bytesWritten, NULL))
	{
		return false;
	}

	if (!WriteFile(Port, pbUpperBuffer, dwPublicKeyBlobLen, &bytesWritten, NULL))
	{
		return false;
	}

	char rsaBuffer[150], rsaUpperBuffer[150];
	DWORD bytesReadRsa;
	if (!ReadFile(Port, rsaBuffer, 148, &bytesReadRsa, NULL))
	{
		return false;
	}

	if (!ReadFile(Port, rsaUpperBuffer, 148, &bytesReadRsa, NULL))
	{
		return false;
	}

	for (unsigned int i = 0; i < bytesReadRsa; ++i)
	{
		if (rsaUpperBuffer[i] == 0x08)
			rsaBuffer[i] = rsaBuffer[i] | 0x80;
	}

	if (!CryptImportKey(hProv, (const BYTE *)rsaBuffer, bytesReadRsa, 0, dwFlags, &opposite_hPubKey))
	{
		return false;
	}

	if (!CryptExportKey(AESKey, opposite_hPubKey, SIMPLEBLOB, 0, NULL, &dwAesKeyBlobLen))
	{
		return false;
	}

	AesBlob = new BYTE[dwAesKeyBlobLen];
	BYTE * AesBlobUpper = new BYTE[dwAesKeyBlobLen];

	if (!CryptExportKey(AESKey, opposite_hPubKey, SIMPLEBLOB, 0, AesBlob, &dwAesKeyBlobLen))
	{
		return false;
	}

	/* Komunikace s druhou stranou a prijem opposite RSA public key */
	DWORD bytesRead = 0;

	if (!CryptGetKeyParam(opposite_hPubKey, KP_BLOCKLEN, NULL, &dwLen, 0))
	{
		return false;
	}

	if (CryptGetKeyParam(opposite_hPubKey, KP_BLOCKLEN, (PBYTE)&dwKeySizeInBits, &dwLen, 0))
	{
		dwBlockSize = dwKeySizeInBits / 8;
	}
	else
	{
		return false;
	}

	BYTE bufferAES[148], upperBufferAES[148];
	DWORD dwAesBytesWrite, dwAesBytesRead;
	for (unsigned int i = 0; i < dwAesKeyBlobLen; ++i)
	{
		if (AesBlob[i] & 0x80)
			AesBlobUpper[i] = 0x08;
	}
	if (!WriteFile(Port, AesBlob, 140, &dwAesBytesWrite, NULL))
	{
		return false;
	}

	if (!WriteFile(Port, AesBlobUpper, 140, &dwAesBytesWrite, NULL))
	{
		return false;
	}

	if (!ReadFile(Port, bufferAES, 140, &dwAesBytesRead, NULL))
	{
		return false;
	}

	if (!ReadFile(Port, upperBufferAES, 140, &dwAesBytesRead, NULL))
	{
		return false;
	}

	for (unsigned int i = 0; i < dwAesBytesRead; ++i)
	{
		if (upperBufferAES[i] == 0x08)
			bufferAES[i] = bufferAES[i] | 0x80;
	}

	if (!CryptImportKey(hProv, bufferAES, dwAesBytesRead, 0, dwFlags, &opposite_AESKey))
	{
		return false;
	}

	delete[] pbPublicBlob;
	delete[] pbUpperBuffer;
	delete[] AesBlob;
	delete[] AesBlobUpper;

	return true;
}

bool EncryptionClass::exchange2(void * Port)
{
	DWORD dwPublicKeyBlobLen, dwAesKeyBlobLen;
	BYTE * pbPublicBlob, *AesBlob, *pbUpperBuffer;

	if (!CryptExportKey(hPubKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwPublicKeyBlobLen))
	{
		return false;
	}

	pbPublicBlob = new BYTE[dwPublicKeyBlobLen];
	pbUpperBuffer = new BYTE[dwPublicKeyBlobLen];

	if (!CryptExportKey(hPubKey, NULL, PUBLICKEYBLOB, 0, pbPublicBlob, &dwPublicKeyBlobLen))
	{
		return false;
	}

	//poslat RSA public part a prijmout oppositeRsaKEy
	char rsaBuffer[150], rsaUpperBuffer[150];
	DWORD bytesReadRsa, bytesWritten;
	WaitForSingleObject(Port, INFINITE);
	if (!ReadFile(Port, rsaBuffer, 148, &bytesReadRsa, NULL))
	{
		return false;
	}

	WaitForSingleObject(Port, INFINITE);
	if (!ReadFile(Port, rsaUpperBuffer, 148, &bytesReadRsa, NULL))
	{
		return false;
	}

	for (unsigned int i = 0; i < bytesReadRsa; ++i)
	{
		if (rsaUpperBuffer[i] == 0x08)
			rsaBuffer[i] = rsaBuffer[i] | 0x80;
	}

	for (unsigned int i = 0; i < dwPublicKeyBlobLen; ++i)
	{
		if (pbPublicBlob[i] & 0x80)
			pbUpperBuffer[i] = 0x08;
	}

	if (!WriteFile(Port, pbPublicBlob, dwPublicKeyBlobLen, &bytesWritten, NULL))
	{
		return false;
	}

	if (!WriteFile(Port, pbUpperBuffer, dwPublicKeyBlobLen, &bytesWritten, NULL))
	{
		return false;
	}

	if (!CryptImportKey(hProv, (const BYTE *)rsaBuffer, bytesReadRsa, 0, dwFlags, &opposite_hPubKey))
	{
		return false;
	}

	if (!CryptExportKey(AESKey, opposite_hPubKey, SIMPLEBLOB, 0, NULL, &dwAesKeyBlobLen))
	{
		return false;
	}

	AesBlob = new BYTE[dwAesKeyBlobLen];
	BYTE * AesBlobUpper = new BYTE[dwAesKeyBlobLen];
	DWORD bytesRead = 0;
	BYTE bufferAES[148], upperBufferAES[148];
	DWORD dwAesBytesWrite, dwAesBytesRead;

	if (!CryptExportKey(AESKey, opposite_hPubKey, SIMPLEBLOB, 0, AesBlob, &dwAesKeyBlobLen))
	{
		return false;
	}

	if (!CryptGetKeyParam(opposite_hPubKey, KP_BLOCKLEN, NULL, &dwLen, 0))
	{
		return false;
	}

	if (CryptGetKeyParam(opposite_hPubKey, KP_BLOCKLEN, (PBYTE)&dwKeySizeInBits, &dwLen, 0))
	{
		dwBlockSize = dwKeySizeInBits / 8;
	}
	else
	{
		return false;
	}

	WaitForSingleObject(Port, INFINITE);
	if (!ReadFile(Port, bufferAES, 140, &dwAesBytesRead, NULL))
	{
		return false;
	}

	WaitForSingleObject(Port, INFINITE);
	if (!ReadFile(Port, upperBufferAES, 140, &dwAesBytesRead, NULL))
	{
		return false;
	}
	for (unsigned int i = 0; i < dwAesBytesRead; ++i)
	{
		if (upperBufferAES[i] == 0x08)
			bufferAES[i] = bufferAES[i] | 0x80;
	}

	for (unsigned int i = 0; i < dwAesKeyBlobLen; ++i)
	{
		if (AesBlob[i] & 0x80)
			AesBlobUpper[i] = 0x08;
	}

	if (!WriteFile(Port, AesBlob, 140, &dwAesBytesWrite, NULL))
	{
		return false;
	}

	if (!WriteFile(Port, AesBlobUpper, 140, &dwAesBytesWrite, NULL))
	{
		return false;
	}

	if (!CryptImportKey(hProv, bufferAES, dwAesBytesRead, 0, dwFlags, &opposite_AESKey))
	{
		return false;
	}

	delete[] pbPublicBlob;
	delete[] pbUpperBuffer;
	delete[] AesBlob;
	delete[] AesBlobUpper;

	return true;
}

char * EncryptionClass::encryptInstruction(char * data, int dataLength)
{
	b_encryptInst = true;
	char * myArrayCrypted = new char[dwBlockSize];
	BYTE * myArray = new BYTE[dwBlockSize];
	DWORD tempLen = dataLength;
	instDataLen = 0;

	for (int i = 0; i < dataLength; ++i)
	{
		myArray[i] = (unsigned char)data[i];
	}

	if (!CryptEncrypt(opposite_hPubKey, NULL, TRUE, 0, myArray, &tempLen, dwBlockSize))
	{
		b_encryptInst = false;
	}

	for (unsigned int i = 0; i < tempLen; ++i)
	{
		myArrayCrypted[i] = myArray[i];
	}

	instDataLen = tempLen;
	delete[] myArray;
	return myArrayCrypted;
}

BYTE * EncryptionClass::decryptInstruction(char * data, int dataLength)
{
	b_decryptInst = true;
	DWORD pbDataLen = dataLength;
	BYTE * dataToDec = new BYTE[dataLength];

	for (int i = 0; i < dataLength; ++i)
	{
		dataToDec[i] = data[i];
	}

	if (!CryptDecrypt(hPubKey, NULL, TRUE, 0, dataToDec, &pbDataLen))
	{
		b_decryptInst = false;
	}
	decInstLength = pbDataLen;

	return dataToDec;
}

char * EncryptionClass::encryptFile(const char * data, int length, bool finalBlock)
{
	b_encryptFile = true;
	char * myArrayEnc = new char[length * 2 + 100];
	BYTE * myArray = new BYTE[length * 2 + 100];
	DWORD tempLen = length;
	for (int i = 0; i < length; ++i)
	{
		myArray[i] = data[i];
	}


	if (!CryptEncrypt(opposite_AESKey, NULL, finalBlock, 0, myArray, &tempLen, length * 2))
	{
		b_encryptFile = false;
	}

	for (unsigned int i = 0; i < tempLen; ++i)
	{
		myArrayEnc[i] = myArray[i];
	}

	fileDataLen = tempLen;
	delete[] myArray;
	return myArrayEnc;
}

BYTE * EncryptionClass::decryptFile(const char * data, int length, bool finalBlock)
{
	b_decryptFile = true;
	DWORD pbDataLen = length;
	BYTE * dataToDec = new BYTE[length + 1];

	for (int i = 0; i < length; ++i)
	{
		dataToDec[i] = data[i];
	}

	if (!CryptDecrypt(AESKey, NULL, finalBlock, 0, dataToDec, &pbDataLen))
	{
		b_decryptFile = false;
	}
	fileDecDataLen = pbDataLen;

	return dataToDec;
}

int EncryptionClass::getInstDataLen()
{
	return instDataLen;
}

int EncryptionClass::getFileDataLen()
{
	return fileDataLen;
}

int EncryptionClass::getFileDecDataLen()
{
	return fileDecDataLen;
}

int EncryptionClass::getInstDecLen()
{
	return decInstLength;
}

bool EncryptionClass::getDecryptFile()
{
	return b_decryptFile;
}

bool EncryptionClass::getEncryptFile()
{
	return b_encryptFile;
}

bool EncryptionClass::getDecryptInst()
{
	return b_decryptInst;
}

bool EncryptionClass::getEncryptInst()
{
	return b_encryptInst;
}