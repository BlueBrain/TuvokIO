#pragma once
#ifndef UVF_H
#define UVF_H

#ifndef UVFVERSION
	#define UVFVERSION 1
#else
	#if UVFVERSION != 1
		>> VERSION MISMATCH <<
	#endif
#endif

#include "UVFTables.h"
#include "DataBlock.h"
#include "GlobalHeader.h"

#include "RasterDataBlock.h"
#include "KeyValuePairDataBlock.h"


class UVF
{
public:
	static UINT64 ms_ulReaderVersion;

	UVF(std::wstring wstrFilename);
	virtual ~UVF(void);

	bool Open(bool bVerify=true, bool bReadWrite=false, std::string* pstrProblem = NULL);
	void Close();

	bool VerifyChecksum(std::string* pstrProblem = NULL);

	const GlobalHeader& GetGlobalHeader() const {return m_GlobalHeader;}
	UINT64 GetDataBlockCount() const {return UINT64(m_DataBlocks.size());}
	const DataBlock* GetDataBlock(UINT64 index) const {return m_DataBlocks[size_t(index)];}

	// file creation routines
	bool SetGlobalHeader(const GlobalHeader& GlobalHeader);
	bool AddDataBlock(DataBlock& dataBlock, UINT64 iSizeofData);
	bool Create();


protected:
  bool			      m_bFileIsLoaded;
	bool			      m_bFileIsReadWrite;
  LargeRAWFile    m_streamFile;

	GlobalHeader m_GlobalHeader;
	std::vector<DataBlock*> m_DataBlocks;

	bool ParseGlobalHeader(bool bVerify, std::string* pstrProblem = NULL);
	void ParseDataBlocks();
	
	std::vector<unsigned char> ComputeChecksum(UVFTables::ChecksumSemanticTable eChecksumSemanticsEntry);

	// file creation routines
	UINT64 ComputeNewFileSize();
	void UpdateChecksum();

};

#endif // UVF_H
