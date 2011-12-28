#include "TOCBlock.h"

#include "MaxMinDataBlock.h"
#include "DebugOut/AbstrDebugOut.h"
#include "ExtendedOctree/ExtendedOctreeConverter.h"

using namespace std;

TOCBlock::TOCBlock() :
  m_iOffsetToOctree(0),
  m_bIsBigEndian(false),
  m_iOverlap(2),
  m_vMaxBrickSize(128,128,128),
  m_strDeleteTempFile("")
{
  ulBlockSemantics = UVFTables::BS_TOC_BLOCK;
  strBlockID       = "Table of Contents Raster Data Block";
}

TOCBlock::TOCBlock(const TOCBlock &other) : 
  DataBlock(other),
  m_bIsBigEndian(other.m_bIsBigEndian)
{
  GetHeaderFromFile(m_pStreamFile, m_iOffset, m_bIsBigEndian);
}

TOCBlock::~TOCBlock(){
  if (!m_strDeleteTempFile.empty() ) {
    m_ExtendedOctree.Close();
    remove(m_strDeleteTempFile.c_str());
  }
}

TOCBlock::TOCBlock(LargeRAWFile_ptr pStreamFile, uint64_t iOffset, bool bIsBigEndian) {
  GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

uint64_t TOCBlock::GetHeaderFromFile(LargeRAWFile_ptr pStreamFile, uint64_t iOffset, bool bIsBigEndian) {
  m_bIsBigEndian = bIsBigEndian;
  m_iOffsetToOctree = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
  m_ExtendedOctree.Open(m_pStreamFile, m_iOffsetToOctree);
  return pStreamFile->GetPos() - iOffset;
}

uint64_t TOCBlock::CopyToFile(LargeRAWFile_ptr pStreamFile, uint64_t iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  CopyHeaderToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);

  uint64_t iDataSize = ComputeDataSize();
  m_pStreamFile->SeekPos(m_iOffsetToOctree);
  unsigned char* pData = new unsigned char[size_t(min(iDataSize, BLOCK_COPY_SIZE))];
  for (uint64_t i = 0;i<iDataSize;i+=BLOCK_COPY_SIZE) {
    uint64_t iCopySize = min(BLOCK_COPY_SIZE, iDataSize-i);

    m_pStreamFile->ReadRAW(pData, iCopySize);
    pStreamFile->WriteRAW(pData, iCopySize);
  }
  delete [] pData;

  return pStreamFile->GetPos() - iOffset;
}

DataBlock* TOCBlock::Clone() const {
  return new TOCBlock(*this);
}

uint64_t TOCBlock::ComputeHeaderSize() const {
  // currently TOC Block contains no header in addition to the
  // header info stored in the ExtendedOctree
  return 0;
}


uint64_t TOCBlock::GetOffsetToNextBlock() const {
  return DataBlock::GetOffsetToNextBlock() + ComputeHeaderSize() + ComputeDataSize();
}

uint64_t TOCBlock::ComputeDataSize() const {
  return m_ExtendedOctree.GetSize();
}

bool TOCBlock::FlatDataToBrickedLOD(const std::string& strSourceFile, const std::string& strTempFile,
                                    ExtendedOctree::COMPONENT_TYPE eType, uint64_t iComponentCount, 
                                    UINT64VECTOR3 vVolumeSize, DOUBLEVECTOR3 vScale, size_t iCacheSize, 
                                    MaxMinDataBlock* pMaxMinDatBlock, AbstrDebugOut* debugOut) {
  LargeRAWFile_ptr inFile(new LargeRAWFile(strSourceFile));
  if (!inFile->Open()) {
    return false;
  }

  return FlatDataToBrickedLOD(inFile, strTempFile, eType, iComponentCount, 
                              vVolumeSize, vScale, iCacheSize, 
                              pMaxMinDatBlock, debugOut);

}

bool TOCBlock::FlatDataToBrickedLOD(LargeRAWFile_ptr pSourceData, const std::string& strTempFile,
                                    ExtendedOctree::COMPONENT_TYPE eType, uint64_t iComponentCount, 
                                    UINT64VECTOR3 vVolumeSize, DOUBLEVECTOR3 vScale, size_t iCacheSize,
                                    MaxMinDataBlock* pMaxMinDatBlock, AbstrDebugOut* ) {
  LargeRAWFile_ptr outFile(new LargeRAWFile(strTempFile));
  if (!outFile->Create()) {
    return false;
  }
  m_pStreamFile = outFile;
  m_strDeleteTempFile = strTempFile;
  ExtendedOctreeConverter c(m_vMaxBrickSize, m_iOverlap, iCacheSize);
  BrickStatVec statsVec;
  bool bResult = c.Convert(pSourceData, 0, eType, iComponentCount, vVolumeSize, vScale, outFile, 0, &statsVec, CT_ZIP);
  outFile->Close();

  if (bResult) {
    pMaxMinDatBlock->SetDataFromFlatVector(statsVec, iComponentCount);
    return m_ExtendedOctree.Open(strTempFile,0);
  } else {
    return false;
  }
}

bool TOCBlock::BrickedLODToFlatData(const std::string& strTargetFile, uint64_t iLoD, AbstrDebugOut* ) const{
  return ExtendedOctreeConverter::ExportToRAW(m_ExtendedOctree,strTargetFile,iLoD,0);  
}

bool TOCBlock::BrickedLODToFlatData(LargeRAWFile_ptr pTargetFile, uint64_t iLoD, AbstrDebugOut* ) const {
  return ExtendedOctreeConverter::ExportToRAW(m_ExtendedOctree,pTargetFile,iLoD,0);  
}

void TOCBlock::GetData(uint8_t* pData, UINT64VECTOR4 coordinates) const {
  m_ExtendedOctree.GetBrickData(pData, coordinates);
}

UINT64VECTOR3 TOCBlock::GetBrickCount(uint64_t iLoD) const {
  return m_ExtendedOctree.GetBrickCount(iLoD);
}

UINTVECTOR3 TOCBlock::GetBrickSize(UINT64VECTOR4 coordinates) const {
  return m_ExtendedOctree.ComputeBrickSize(coordinates);
}

UINT64VECTOR3 TOCBlock::GetLODDomainSize(uint64_t iLoD) const {
  return m_ExtendedOctree.GetLoDSize(iLoD);
}

uint64_t TOCBlock::GetLoDCount() const {
  return m_ExtendedOctree.GetLODCount();
}