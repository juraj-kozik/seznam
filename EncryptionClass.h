#pragma once

#include <Windows.h>

class EncryptionClass
{
public:
	EncryptionClass();
	~EncryptionClass();
	bool setup(void * Port);
	bool exchange1(void * Port);
	bool exchange2(void * Port);
	char * encryptInstruction(char * data, int dataLength);
	BYTE * decryptInstruction(char * data, int dataLength);
	char * encryptFile(const char * data, int length, bool finalBlock);
	BYTE * decryptFile(const char * data, int length, bool finalBlock);
	int getInstDataLen();
	int getFileDataLen();
	int getFileDecDataLen();
	int getInstDecLen();
	bool getDecryptFile();
	bool getEncryptFile();
	bool getDecryptInst();
	bool getEncryptInst();
private:
	HCRYPTPROV hProv;

	HCRYPTKEY hPubKey;
	HCRYPTKEY opposite_hPubKey;
	HCRYPTKEY AESKey;
	HCRYPTKEY opposite_AESKey;

	DWORD dwFlags = 0;
	DWORD dwBlockSize;
	DWORD dwKeySizeInBits;
	DWORD dwLen;

	int instDataLen;
	int fileDataLen;
	int fileDecDataLen;
	int decInstLength;

	bool b_decryptFile;
	bool b_encryptFile;
	bool b_decryptInst;
	bool b_encryptInst;

	bool exchanged;
};

