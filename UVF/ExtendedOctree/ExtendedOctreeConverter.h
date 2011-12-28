/*
 The MIT License
 
 Copyright (c) 2011 Interactive Visualization and Data Analysis Group
 
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

#pragma once

#ifndef EXTENDEDOCTREECONVERTER_H
#define EXTENDEDOCTREECONVERTER_H

#include <cstring>
#include <functional>
#include "ExtendedOctree.h"

/*! \brief A single brick cache entry
 *
 *  This class is used in a vector/list etc. like data structure within
 *  the ExtendedOctreeConverter class it mainly stores an aray with the
 *  brick data but also contains an access counter for the FIFO implementation,
 *  a dirty bool to indicate that this brick has changed in mem but has not
 *  yet written to disk, and index indicating to which brick the data belongs
 */
class CacheEntry {
public:  
  /**
    default constructor, flags this CacheEntry as unused
  */
  CacheEntry() :
    m_pData(NULL),
    m_bDirty(false),
    m_index(std::numeric_limits<size_t>::max()),
    m_iAccess(0),
    m_size(0)
  {}

  /**
    cleanup memory if this cache entry was ever active
  */
  ~CacheEntry() {
    delete [] m_pData;    
  }

  /**
    Specify the memory size of this cache block, does not allocate memory yet
    but may delete memory if previous size is different, shall never be called
    on a dirty cache entry

    @param size the size of the block
  */
  void SetSize(size_t size) {
    if (m_size != size) {
      assert(!m_bDirty);
      delete [] m_pData;
      m_pData = NULL;
    }

    m_size = size;
  }
  
  /**
    Actually allocates the memory specified with the size
  */
  void Allocate() {
    delete [] m_pData;
    m_pData = new uint8_t[m_size];    
  }
  
  /// the data pointer 
  uint8_t* m_pData;

  /// true iff the cache entry has been changed but the changes have not yet been committed
  bool m_bDirty;

  /// the ID of the data stored in this cache entry
  size_t m_index;

  /// access timestamp for the replacement strategy
  uint64_t m_iAccess;

private:
  /// the size of the data block
  size_t m_size;
};

/// The brick cache is nothing but a vector of cache elements, we may
/// want to change that to a more complex data structure
typedef std::vector<CacheEntry> BrickCache;

/// Brick cache iterator
typedef BrickCache::iterator BrickCacheIter;

/*! \brief Stores brick statistics such as the minimum and maximum values
 */
template<class T> class BrickStats {
public:
     BrickStats() :
        minScalar(0),
        maxScalar(0)
      {}

      BrickStats(T _minScalar, T _maxScalar) :
        minScalar(_minScalar),
        maxScalar(_maxScalar)
      {}

      T minScalar;
      T maxScalar;
};


/// Vector to store statistics of each brick
typedef std::vector< BrickStats< double > > BrickStatVec;

/*! \brief A class that takes a volume as a 1D array and
 *         turns it into a bricked, hierarchical Extended octree
 *
 */
class ExtendedOctreeConverter {
public:

  /**
    Default constructor, which takes memory management parameters

    @param vBrickSize the maximum size of a brick (including overlap)
    @param iOverlap the voxel overlap (must be smaller than half the brick size in all dimensions)
    @param iMemLimit the amount of memory in bytes the converter is allowed to use for caching
  */
  ExtendedOctreeConverter(const UINTVECTOR3& vBrickSize, 
                          uint32_t iOverlap, uint64_t iMemLimit) : 
      m_fProgress(0.0f),
      m_vBrickSize(vBrickSize), 
      m_iOverlap(iOverlap),
      m_iMemLimit(iMemLimit),
      m_iCacheAccessCounter(0),
      m_pBrickStatVec(NULL)
      {}

  /**
    This call starts the conversion process of a simple linear file of raw volume
    data into a bricked hierarchy, metadata us supplied as parameters

    @param filename the source data file
    @param iOffset the offset of the raw data in the source file
    @param eComponentType the type of data stored (e.g. UINT8 for 8bit unsigned char data)
    @param iComponentCount the vector length of a voxel (e.g. 1 for scalar data or 3 for RGB)
    @param vVolumeSize the dimensions of the input volume
    @param vVolumeAspect the aspect ratio of the input volume 
    @param targetFile the target file for the processed data
    @param iOutOffset bytes to precede the data in the target file
    @param stats pointer to a vector to store the statistics of each brick, can be set to NULL to disable statistics computation
    @param compression the desired compression method, defaults to none (=CT_NONE)
    @return  true if the conversion succeeded, the main reason for failure would be a disk I/O issue
  */
  bool Convert(const std::string& filename, uint64_t iOffset, 
               ExtendedOctree::COMPONENT_TYPE eComponentType, 
               uint64_t iComponentCount, const UINT64VECTOR3& vVolumeSize, 
               const DOUBLEVECTOR3& vVolumeAspect,
               const std::string& targetFile, uint64_t iOutOffset, 
               BrickStatVec* stats = NULL,
               COMPORESSION_TYPE compression=CT_NONE);

  /**
    This call starts the conversion process of a simple linear file of raw volume
    data into a bricked hierarchy, metadata us supplied as parameters

    @param pLargeRAWFile a large raw-file pointer to the source file
    @param iOffset the offset of the raw data in the source file
    @param eComponentType the type of data stored (e.g. UINT8 for 8bit unsigned char data)
    @param iComponentCount the vector length of a voxel (e.g. 1 for scalar data or 3 for RGB)
    @param vVolumeSize the dimensions of the input volume
    @param vVolumeAspect the aspect ratio of the input volume 
    @param pLargeRAWOutFile a large raw-file pointer to the target file for the processed data
    @param iOutOffset bytes to precede the data in the target file
    @param stats pointer to a vector to store the statistics of each brick, can be set to NULL to disable statistics computation
    @param compression the desired compression method, defaults to none (=CT_NONE)
    @return  true if the conversion succeeded, the main reason for failure would be a disk I/O issue
  */
  bool Convert(LargeRAWFile_ptr pLargeRAWFile, uint64_t iOffset, 
               ExtendedOctree::COMPONENT_TYPE eComponentType, 
               uint64_t iComponentCount, const UINT64VECTOR3& vVolumeSize,
               const DOUBLEVECTOR3& vVolumeAspect, 
               LargeRAWFile_ptr pLargeRAWOutFile, uint64_t iOutOffset,
               BrickStatVec* stats = NULL,
               COMPORESSION_TYPE compression= CT_NONE);
  /**
    Call this method from a second thread during the conversion to check on the progress of the operation
  */
  float GetProgress() const {return m_fProgress;}


  /**
   Exports a specific LoD Level into a continuous raw file 
   @param filename the file to be written to, any existing data is overridden
   @param iLODLevel the level to be exported
   @param  iOffset the bytes to be skipped from the beginning of the file
   @return true iff the export was successful
   */
  static bool ExportToRAW(const ExtendedOctree &tree, const std::string& filename, uint64_t iLODLevel, uint64_t iOffset);

  /**
   Exports a specific LoD Level into a continuous raw file 

   @param pointer to a LargeRAW file, file needs to be open, any existing data is overridden
   @param iLODLevel the level to be exported
   @param  iOffset the bytes to be skipped from the beginning of the file
   @return true iff the export was successful
   */
  static bool ExportToRAW(const ExtendedOctree &tree, LargeRAWFile_ptr pLargeRAWFile, uint64_t iLODLevel, uint64_t iOffset);


private:
  /// internal data for the progress indicator call
  float m_fProgress;

  /// the maximum brick size allowed (including overlap) e.g. the usable size is in x is m_vBrickSize.x-2*m_iOverlap
  UINTVECTOR3 m_vBrickSize;

  /// the brick overlap 
  uint32_t m_iOverlap;

  /// max amount of memory in bytes to be used by the cache
  uint64_t m_iMemLimit;

  /// desired compression method for new bricks, may be ignored by the system 
  /// e.g. when a compressed brick would be larger than the uncompressed
  COMPORESSION_TYPE m_eCompression;

  /// the brick cache collection
  BrickCache m_vBrickCache;

  /// last timestamp used to access the brickCache
  uint64_t m_iCacheAccessCounter;

  /// if not NULL then the statistics for each brick are stored in this vector
  BrickStatVec* m_pBrickStatVec;

  /**
    Compresses the bricks using the method specified in m_eCompression
    staring with index iBrickSkip

    @param tree the Extended Octree containing the bricks to be compressed
    @param iBrickSkip the bricks to be skipped, i.e. iBrickSkip is the first index to be compressed
  */
  void Compress(ExtendedOctree &tree, size_t iBrickSkip);

  /**
    Copies (parts) of one brick into another

    @param vSourceData pointer to the source brick data
    @param sourceBrickSize the size of the source brick
    @param vTargetData pointer to the target brick data
    @param targetBrickSize the size of the target brick
    @param sourceOffset coordinates in the source brick were to start copying
    @param targetOffset coordinates in the target brick were to start copying
    @param regionSize x,y,z extension of the region to be copied
    @param voxelSize the size (in bytes) of a brick voxel (i.e. component-size*component-count)
  */
  void CopyBrickToBrick(std::vector<uint8_t>& vSourceData, 
                        const UINTVECTOR3& sourceBrickSize,
                        std::vector<uint8_t>& vTargetData, 
                        const UINTVECTOR3& targetBrickSize,
                        const UINTVECTOR3& sourceOffset, 
                        const UINTVECTOR3& targetOffset,
                        const UINTVECTOR3& regionSize,
                        size_t voxelSize);

  /**
    Fetches a brick from the raw linear input file

    @param vData vector to store the brick data
    @param tree target extended octree (used to extract metadata)
    @param pLargeRAWFileIn source raw file
    @param iInOffset offset into the source file
    @param coords brick coordinates of the brick to be extracted
  */
  void GetInputBrick(std::vector<uint8_t>& vData, 
                     ExtendedOctree &tree, LargeRAWFile_ptr pLargeRAWFileIn,
                     uint64_t iInOffset, const UINT64VECTOR4& coords);

  /**
    This method reorders the large input raw file into smaller bricks
    of maximum size m_vBrickSize with an overlap of m_iOverlap i.e.
    it computes LoD level zero

    @param tree target extended octree
    @param pLargeRAWFileIn source raw file
    @param iInOffset offset into the source file
  */
  void PermuteInputData(ExtendedOctree &tree,
                        LargeRAWFile_ptr pLargeRAWFileIn, uint64_t iInOffset);

  /**
    This method fills the overlaps between the bricks, it assumes
    that the "inner" parts of the bricks have been completed already

    @param tree target extended octree
    @param iLoD the level of detail to be processed
  */
  void FillOverlap(ExtendedOctree &tree, uint64_t iLoD);

  /**
    Loads a specific brick from disk (or cache) into pData

    @param pData the target buffer of the brick data
    @param tree target extended octree
    @param vBrickCoords the coordinates (x,y,z, LoD) of the requested brick
  */ 
  void GetBrick(uint8_t* pData, const ExtendedOctree &tree,
                const UINT64VECTOR4& vBrickCoords);

  /**
    Loads a specific brick from disk (or cache) into pData

    @param pData the target buffer of the brick data
    @param tree target extended octree
    @param index the 1D-index of the brick
  */
  void GetBrick(uint8_t* pData, const ExtendedOctree &tree, uint64_t index);

  /**
    Stores a specific brick to disk (or cache) from pData

    @param pData the source buffer of the brick data
    @param tree target extended octree
    @param vBrickCoords the coordinates (x,y,z, LoD) of the requested brick
  */ 
  void SetBrick(uint8_t* pData, ExtendedOctree &tree, 
                const UINT64VECTOR4& vBrickCoords);

  /**
    Stores a specific brick to disk (or cache) from pData

    @param pData the source buffer of the brick data
    @param tree target extended octree
    @param index the 1D-index of the brick
  */
  void SetBrick(uint8_t* pData, ExtendedOctree &tree, uint64_t index);

  /**
    Prepares the cache data structures, effectively resizes the 
    std collection and notifies all elements of the maximum brick size

    @param tree target extended octree
  */
  void SetupCache(ExtendedOctree &tree);

  /**
    Writes all dirty cache elements to disk

    @param tree target extended octree
  */
  void FlushCache(ExtendedOctree &tree);
  
  /**
    Write a single brick at index i in the ToC to disk and updates
    the minmax data structure if it is set

    @param tree target extended octree
    @param element iterator to the element in the cache to be written to disk
  */
  void WriteBrickToDisk(const ExtendedOctree &tree, BrickCacheIter element);

  /**
    Write a single brick at index i in the ToC to disk and updates
    the minmax data structure if it is set

    @param tree target extended octree
    @param pData the data of the brick
    @param index index of the brick to be written
  */
  void WriteBrickToDisk(const ExtendedOctree &tree, uint8_t* pData, size_t index);

  /**
    Computes the mean value of inputs a to h
    intermediate computations are carried out in double precision to avoid 
    clamping and-or quantization. This function is used for the majority 
    of values when downsampling the bricks, only when no neighbors are present
    in one or multiple directions are the other Average functions (with 4 
    and 2 parameters) called

    @param a value to be averaged
    @param b value to be averaged
    @param c value to be averaged
    @param d value to be averaged
    @param e value to be averaged
    @param f value to be averaged
    @param g value to be averaged
    @param h value to be averaged
    @return the average/mean of values a to h
  */
  template<class T> T Average(T a, T b, T c, T d, T e, T f, T g, T h);
  
  /**
    Computes the mean value of inputs a to d
    intermediate computations are carried out in double precision to avoid 
    clamping and-or quantization. This function is used for if neighbors in
    one dimension are missing 

    @param a value to be averaged
    @param b value to be averaged
    @param c value to be averaged
    @param d value to be averaged
    @return the average/mean of values a to d
  */
  template<class T> T Average(T a, T b, T c, T d);

  /**
    Computes the mean value (a+b)/2 of a and b
    intermediate computations are carried out in double precision to avoid 
    clamping and-or quantization. This function is used for if neighbors in
    two dimensions are missing 

    @param a value to be averaged
    @param b value to be averaged
    @return the average/mean of values a and b 
  */
  template<class T> T Average(T a, T b);

  /**
    This function takes ONE source brick and down-samples this one into the appropriate position
    into target brick the i.e. this function must be called up to eight times, 
    depending on the position, to complete the down sampling process of one target brick

    @param tree target extended octree
    @param pData pointer to the target data
    @param targetSize size of the target brick
    @param pSourceData pointer to the source data
    @param sourceCoords brick coordinates of the source brick
    @param targetOffset coordinates were to place the down-sampled data in the target brick
  */
  template<class T> void DownsampleBricktoBrick(ExtendedOctree &tree, T* pData,
                                                const UINTVECTOR3& targetSize,
                                                T* pSourceData,
                                                const UINT64VECTOR4& sourceCoords,
                                                const UINTVECTOR3& targetOffset);  

  /**
    This function down-samples up to eight bricks into a single brick.
    to avoid new/delete calls this function takes two points to two
    arrays of sufficient size to hold the largest bricks

    @param tree target extended octree
    @param vBrickCoords brick coordinates of the target brick of the downsampling
    @param pData pointer to hold the temp data during the downsampling process
    @param pSourceData pointer to hold the temp data during the downsampling process
  */
  template<class T> void DownsampleBrick(ExtendedOctree &tree, 
                                         const UINT64VECTOR4& vBrickCoords, 
                                         T* pData, T* pSourceData);

  /**
    This function computes all the LoD levels on
    top of the highest resolution (level 0)

    @param tree target extended octree
  */
  template<class T> void ComputeHirarchy(ExtendedOctree &tree);


  /**
    Computes the statistics of an array

    @param pData pointer to the array
    @param iLength size (IN BYTES) of the array
    @param iComponentCount number of components per voxel
  */
  template<class T> BrickStatVec ComputeBrickStats(uint8_t* pData, uint64_t iLength, size_t iComponentCount);
};

/// this file contains all the implementations of the template functions 
/// used in the ExtendedOctreeConverter
#include "ExtendedOctreeConverter.inc"

#endif // EXTENDEDOCTREECONVERTER_H