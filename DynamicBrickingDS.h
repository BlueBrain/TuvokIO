#ifndef TUVOK_DYNAMIC_BRICKING_DS_H
#define TUVOK_DYNAMIC_BRICKING_DS_H

#include <array>
#include <memory>
#include <vector>
#include "BrickedDataset.h"

namespace tuvok {

/// A dataset which will dynamically break up another data set into the
/// user-given brick sizes.

/// During construction, you must give an already-opened data set and
/// the desired brick size.
class DynamicBrickingDS : public BrickedDataset {
public:
  DynamicBrickingDS(std::shared_ptr<Dataset> ds,
                    std::array<unsigned, 3> maxBrickSize);
  virtual ~DynamicBrickingDS();

  virtual float MaxGradientMagnitude() const;
  /// Removes all the cache information we've made so far.
  virtual void Clear();

  virtual UINTVECTOR3 GetBrickVoxelCounts(const BrickKey&) const;

  /// Data access
  ///@{
  virtual bool GetBrick(const BrickKey&, std::vector<uint8_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<int8_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<uint16_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<int16_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<uint32_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<int32_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<float>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<double>&) const;
  ///@}

  /// User rescaling factors.
  ///@{
  void SetRescaleFactors(const DOUBLEVECTOR3&);
  DOUBLEVECTOR3 GetRescaleFactors() const;
  /// If the underlying file format supports it, save the current scaling
  /// factors to the file.  The format should implicitly load and apply the
  /// scaling factors when opening the data set.
  virtual bool SaveRescaleFactors();
  ///@}

  virtual uint64_t GetLODLevelCount() const;
  virtual uint64_t GetNumberOfTimesteps() const;
  virtual UINT64VECTOR3 GetDomainSize(const size_t lod=0,
                                      const size_t ts=0) const;
  virtual UINTVECTOR3 GetBrickOverlapSize() const;
  UINT64VECTOR3 GetEffectiveBrickSize(const BrickKey&) const;

  virtual unsigned GetBitWidth() const;
  virtual uint64_t GetComponentCount() const;
  virtual bool GetIsSigned() const;
  virtual bool GetIsFloat() const;
  virtual bool IsSameEndianness() const;
  virtual std::pair<double,double> GetRange() const;

  /// Acceleration queries.
  virtual bool ContainsData(const BrickKey&, double /*isoval*/) const;
  virtual bool ContainsData(const BrickKey&, double /*fMin*/,
                            double /*fMax*/) const;
  virtual bool ContainsData(const BrickKey&, double /*fMin*/, double /*fMax*/,
                            double /*fMinGradient*/,
                            double /*fMaxGradient*/) const;

  /// unimplemented!  Override these if you want tools built on this IO layer
  /// to be able to create data in your format.
  virtual bool Export(uint64_t iLODLevel, const std::string& targetFilename,
                      bool bAppend) const;

  virtual bool ApplyFunction(uint64_t iLODLevel, 
                        bool (*brickFunc)(void* pData, 
                                          const UINT64VECTOR3& vBrickSize,
                                          const UINT64VECTOR3& vBrickOffset,
                                          void* pUserContext),
                        void *pUserContext,
                        uint64_t iOverlap) const;

  /// A user-visible name for your format.  This might get displayed in UI
  /// elements; e.g. the GUI might ask if the user wants to use the "Name()
  /// reader" to open a particular file.
  virtual const char* Name() const;
  /// Virtual constructor.
  virtual DynamicBrickingDS* Create(const std::string&, uint64_t, bool) const;

  /// this function computes the texture coordinates for a given brick
  /// this may be non trivial with power of two padding, overlap handling
  /// and per brick rescale
  virtual std::pair<FLOATVECTOR3, FLOATVECTOR3>
  GetTextCoords(BrickTable::const_iterator brick,
                bool bUseOnlyPowerOfTwo) const;

private:
  // rebricks the data according to the current brick size parameters.
  void Rebrick();

private:
  struct dbinfo;
  std::unique_ptr<struct dbinfo> di;

};

}
#endif // TUVOK_DATASET_H
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Thomas Fogal


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