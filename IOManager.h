/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    IOManager.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/
#pragma once

#ifndef IOMANAGER_H
#define IOMANAGER_H

#include "StdTuvokDefines.h"
#ifdef _MSC_VER
# include <memory>
# include <tuple>
#else
# include <tr1/memory>
# include <tr1/tuple>
#endif
#include <algorithm>
#include <fstream>
#include <limits>
#include <list>
#include <string>
#include "Controller/MasterController.h"
#include "Basics/MC.h"
#include "Basics/SysTools.h"
#include "Basics/LargeRAWFile.h"
#include "Basics/Mesh.h"
#include "Basics/TuvokException.h"
#include "TransferFunction1D.h"
#include "AbstrGeoConverter.h"

typedef std::tr1::tuple<std::string,std::string,bool,bool> tConverterFormat;

#define DEFAULT_BRICKSIZE (256)
#define DEFAULT_BRICKOVERLAP (2)
#define DEFAULT_INCORESIZE (DEFAULT_BRICKSIZE*DEFAULT_BRICKSIZE*DEFAULT_BRICKSIZE)

class AbstrConverter;
class FileStackInfo;
class RangeInfo;
class UVF;
class GeometryDataBlock;

namespace tuvok {
  class AbstrRenderer;
  class Dataset;
  class FileBackedDataset;
  class UVFDataset;
  class MasterController;
  namespace io {
    class DSFactory;
  }
}

class MergeDataset {
public:
  MergeDataset(std::string _strFilename="", uint64_t _iHeaderSkip=0, bool _bDelete=false,
               double _fScale=1.0, double _fBias=0.0) :
    strFilename(_strFilename),
    iHeaderSkip(_iHeaderSkip),
    bDelete(_bDelete),
    fScale(_fScale),
    fBias(_fBias)
  {}

  std::string strFilename;
  uint64_t iHeaderSkip;
  bool bDelete;
  double fScale;
  double fBias;
};

class IOManager {
public:
  IOManager();
  ~IOManager();

  std::vector<std::tr1::shared_ptr<FileStackInfo> >
    ScanDirectory(std::string strDirectory) const;
  bool ConvertDataset(FileStackInfo* pStack,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      uint64_t iMaxBrickSize,
                      uint64_t iBrickOverlap,
                      bool bQuantizeTo8Bit=false) const;
  bool ConvertDataset(const std::string& strFilename,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const bool bNoUserInteraction,
                      uint64_t iMaxBrickSize,
                      uint64_t iBrickOverlap,
                      bool bQuantizeTo8Bit=false) const;
  bool ConvertDataset(const std::list<std::string>& files,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const bool bNoUserInteraction,
                      uint64_t iMaxBrickSize,
                      uint64_t iBrickOverlap,
                      bool bQuantizeTo8Bit=false) const;
  bool MergeDatasets(const std::vector <std::string>& strFilenames,
                     const std::vector <double>& vScales,
                     const std::vector<double>& vBiases,
                     const std::string& strTargetFilename,
                     const std::string& strTempDir,
                     bool bUseMaxMode=true,
                     bool bNoUserInteraction=false) const;
  tuvok::UVFDataset* ConvertDataset(FileStackInfo* pStack,
                                    const std::string& strTargetFilename,
                                    const std::string& strTempDir,
                                    tuvok::AbstrRenderer* requester,
                                    uint64_t iMaxBrickSize,
                                    uint64_t iBrickOverlap,
                                    bool bQuantizeTo8Bit=false) const;
  tuvok::UVFDataset* ConvertDataset(const std::string& strFilename,
                                    const std::string& strTargetFilename,
                                    const std::string& strTempDir,
                                    tuvok::AbstrRenderer* requester,
                                    uint64_t iMaxBrickSize,
                                    uint64_t iBrickOverlap,
                                    bool bQuantizeTo8Bit=false) const;

  /// evaluates the given expression. v[n] in the expression refers to
  /// the volume given by volumes[n].
  void EvaluateExpression(const char* expr,
                          const std::vector<std::string> volumes,
                          const std::string& out_fn) const
                          throw(tuvok::Exception);


  bool ReBrickDataset(const std::string& strSourceFilename,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const uint64_t iMaxBrickSize,
                      const uint64_t iBrickOverlap,
                      bool bQuantizeTo8Bit=false) const;

  // convenience calls that use the default bricksizes and overlaps
  tuvok::UVFDataset* ConvertDataset(FileStackInfo* pStack,
                                    const std::string& strTargetFilename,
                                    const std::string& strTempDir,
                                    tuvok::AbstrRenderer* requester,
                                    const bool bQuantizeTo8Bit=false) const {
    return ConvertDataset(pStack,strTargetFilename,strTempDir,requester,m_iMaxBrickSize, m_iBrickOverlap, bQuantizeTo8Bit);
  }
  tuvok::UVFDataset* ConvertDataset(const std::string& strFilename,
                                    const std::string& strTargetFilename,
                                    const std::string& strTempDir,
                                    tuvok::AbstrRenderer* requester,
                                    const bool bQuantizeTo8Bit=false) {
    return ConvertDataset(strFilename,strTargetFilename,strTempDir,requester,m_iMaxBrickSize, m_iBrickOverlap,bQuantizeTo8Bit);
  }
  bool ConvertDataset(FileStackInfo* pStack,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const bool bQuantizeTo8Bit=false) const{
    return ConvertDataset(pStack,strTargetFilename,strTempDir,m_iMaxBrickSize, m_iBrickOverlap, bQuantizeTo8Bit);
  }
  bool ConvertDataset(const std::string& strFilename,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const bool bNoUserInteraction=false,
                      const bool bQuantizeTo8Bit=false) const {
    return ConvertDataset(strFilename,strTargetFilename,strTempDir,bNoUserInteraction,m_iMaxBrickSize,m_iBrickOverlap, bQuantizeTo8Bit);
  }
  bool ConvertDataset(const std::list<std::string>& files,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      const bool bNoUserInteraction=false,
                      const bool bQuantizeTo8Bit=false) const {
    return ConvertDataset(files,strTargetFilename,strTempDir,bNoUserInteraction,m_iMaxBrickSize,m_iBrickOverlap, bQuantizeTo8Bit);
  }
  bool ReBrickDataset(const std::string& strSourceFilename,
                      const std::string& strTargetFilename,
                      const std::string& strTempDir,
                      bool bQuantizeTo8Bit=false) const {
    return ReBrickDataset(strSourceFilename,strTargetFilename,strTempDir,m_iMaxBrickSize,m_iBrickOverlap,bQuantizeTo8Bit);
  }

  tuvok::Mesh* LoadMesh(const std::string& meshfile) const;

  void AddMesh(const UVF* sourceDataset,
                  const std::string& trisoup_file,
                  const std::string& uvf) const;

  tuvok::Dataset* LoadDataset(const std::string& strFilename,
                              tuvok::AbstrRenderer* requester) const;
  tuvok::Dataset* CreateDataset(const std::string& filename,
                                uint64_t max_brick_size, bool verify) const;
  void AddReader(std::tr1::shared_ptr<tuvok::FileBackedDataset>);
  bool AnalyzeDataset(const std::string& strFilename, RangeInfo& info,
                      const std::string& strTempDir) const;
  bool NeedsConversion(const std::string& strFilename) const;
  bool Verify(const std::string& strFilename) const;

  bool ExportMesh(const tuvok::Mesh* mesh, const std::string& strTargetFilename);
  bool ExportDataset(const tuvok::UVFDataset* pSourceData, uint64_t iLODlevel,
                     const std::string& strTargetFilename,
                     const std::string& strTempDir) const;
  bool ExtractIsosurface(const tuvok::UVFDataset* pSourceData,
                         uint64_t iLODlevel, double fIsovalue,
                         const FLOATVECTOR4& vfColor,
                         const std::string& strTargetFilename,
                         const std::string& strTempDir) const;
  bool ExtractImageStack(const tuvok::UVFDataset* pSourceData,
                         const TransferFunction1D* pTrans,
                         uint64_t iLODlevel, 
                         const std::string& strTargetFilename,
                         const std::string& strTempDir,
                         bool bAllDirs) const;


  void RegisterExternalConverter(AbstrConverter* pConverter);
  void RegisterFinalConverter(AbstrConverter* pConverter);

  std::string GetLoadDialogString() const;
  std::string GetExportDialogString() const;
  std::string GetImageExportDialogString() const;


  std::vector< std::pair <std::string, std::string > >
    GetImportFormatList() const;
  std::vector< std::pair <std::string, std::string > >
    GetExportFormatList() const;
  std::vector< tConverterFormat > GetFormatList() const;
  AbstrConverter* GetConverterForExt(std::string ext,
                                     bool bMustSupportExport,
                                     bool bMustSupportImport) const;

  std::string GetLoadGeoDialogString() const;
  std::string GetGeoExportDialogString() const;
  std::vector< std::pair <std::string, std::string > >
    GetGeoImportFormatList() const;
  std::vector< std::pair <std::string, std::string > >
    GetGeoExportFormatList() const;
  std::vector< tConverterFormat > GetGeoFormatList() const;
  tuvok::AbstrGeoConverter* GetGeoConverterForExt(std::string ext,
                                                  bool bMustSupportExport,
                                                  bool bMustSupportImport) const;


  uint64_t GetMaxBrickSize() const {return m_iMaxBrickSize;}
  uint64_t GetBrickOverlap() const {return m_iBrickOverlap;}
  uint64_t GetIncoresize() const {return m_iIncoresize;}

  bool SetMaxBrickSize(const uint64_t iMaxBrickSize);
  bool SetBrickOverlap(const uint64_t iBrickOverlap);

private:
  std::vector<tuvok::AbstrGeoConverter*> m_vpGeoConverters;
  std::vector<AbstrConverter*>    m_vpConverters;
  AbstrConverter*                 m_pFinalConverter;
  std::auto_ptr<tuvok::io::DSFactory> m_dsFactory;

  uint64_t m_iMaxBrickSize;
  uint64_t m_iBrickOverlap;
  uint64_t m_iIncoresize;

  void CopyToTSB(const tuvok::Mesh* m, GeometryDataBlock* tsb) const;
};

#endif // IOMANAGER_H
