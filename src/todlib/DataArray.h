#ifndef __DATAARRAY_H__
#define __DATAARRAY_H__

#include "TodCommon.h"
#include "TodDebug.h"

enum {
    DATA_ARRAY_INDEX_MASK = 65535,
    DATA_ARRAY_KEY_MASK = -65536,
    DATA_ARRAY_KEY_SHIFT = 16,
    DATA_ARRAY_MAX_SIZE = 65536,
    DATA_ARRAY_KEY_FIRST = 1
};

template <typename T>
struct DataArrayItem {
    T mItem;
    size_t mID;
};

template <typename T> class DataArray {
public:
    using DataArrayItem = DataArrayItem<T>;


public:
    std::vector<DataArrayItem> mBlock;
    size_t mMaxUsedCount;
    size_t mMaxSize;
    size_t mFreeListHead;
    size_t mSize;
    size_t mNextKey;
    std::string mName{};

public:
    DataArray<T>() {
        mMaxUsedCount = 0U;
        mMaxSize = 0U;
        mFreeListHead = 0U;
        mSize = 0U;
        mNextKey = 1U;
    }

    ~DataArray<T>() { DataArrayDispose(); }

    void DataArrayInitialize(const size_t theMaxSize, const std::string &theName) {
        mMaxSize = theMaxSize;
        mBlock.resize(mMaxSize);
        mNextKey = 1001U;
        mName = theName;
    }

    void DataArrayDispose() {
        DataArrayFreeAll();
        mMaxUsedCount = 0U;
        mMaxSize = 0U;
        mFreeListHead = 0U;
        mSize = 0U;
    }

    void DataArrayFree(T *theItem) {
        DataArrayItem *aItem = (DataArrayItem *)theItem;
        TOD_ASSERT(DataArrayGet(aItem->mID) == theItem, "Failed: DataArrayFree(0x{0:x}) in {}", theItem, mName);
        theItem->~T();
        unsigned int anId = aItem->mID & DATA_ARRAY_INDEX_MASK;
        aItem->mID = mFreeListHead;
        mFreeListHead = anId;
        mSize--;
    }

    void DataArrayFreeAll() {
        T *aItem = nullptr;
        while (IterateNext(aItem))
            DataArrayFree(aItem);

        mFreeListHead = 0U;
        mMaxUsedCount = 0U;
    }

    inline unsigned int DataArrayGetID(T *theItem) {
        DataArrayItem *aItem = (DataArrayItem *)theItem;
        TOD_ASSERT(DataArrayGet(aItem->mID) == theItem, "Failed: DataArrayGetID(0x{0:x}) for {}", theItem, mName);
        return aItem->mID;
    }

    bool IterateNext(T *&theItem) {
        DataArrayItem *aItem = (DataArray<T>::DataArrayItem *)theItem;
        if (aItem == nullptr) aItem = &mBlock[0];
        else ++aItem;

        DataArrayItem *aLast = &mBlock[mMaxUsedCount];
        while (aItem < aLast) {
            if (aItem->mID & DATA_ARRAY_KEY_MASK) {
                theItem = (T *)aItem;
                return true;
            }
            aItem++;
        }
        return false;
    }

    T *DataArrayAlloc() {
        TOD_ASSERT(mSize < mMaxSize, "Data array full: {}", mName);
        TOD_ASSERT(mFreeListHead <= mMaxUsedCount, "DataArrayAlloc error in {}", mName);
        unsigned int aNext = mMaxUsedCount;
        if (mFreeListHead == mMaxUsedCount) mFreeListHead = ++mMaxUsedCount;
        else {
            aNext = mFreeListHead;
            mFreeListHead = mBlock[mFreeListHead].mID;
        }

        DataArray<T>::DataArrayItem *aNewItem = &mBlock[aNext];
        memset(aNewItem, 0, sizeof(DataArrayItem));
        aNewItem->mID = (mNextKey++ << DATA_ARRAY_KEY_SHIFT) | aNext;
        if (mNextKey == DATA_ARRAY_MAX_SIZE) mNextKey = 1;
        mSize++;

        new (aNewItem) T();
        return (T*)aNewItem;
    }

    T *DataArrayTryToGet(unsigned int theId) {
        if (!theId || (theId & DATA_ARRAY_INDEX_MASK) >= mMaxSize) return nullptr;

        DataArrayItem *aBlock = &mBlock[theId & DATA_ARRAY_INDEX_MASK];
        return (aBlock->mID == theId) ? &aBlock->mItem : nullptr;
    }

    T *DataArrayGet(unsigned int theId) {
        TOD_ASSERT(DataArrayTryToGet(theId) != nullptr, "Failed: DataArrayGet(0x{0:x}}) for {}", theId, mName);
        return &mBlock[(short)theId].mItem;
    }
};

#endif
