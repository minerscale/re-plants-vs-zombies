#include "TodList.h"
#include "TodCommon.h"
#include "TodDebug.h"
#include "misc/Debug.h"

void TodAllocator::Initialize(int theGrowCount, int theItemSize) {
    TOD_ASSERT(static_cast<size_t>(theItemSize) >= sizeof(void *));

    mFreeList = nullptr;
    mBlockList = nullptr;
    mGrowCount = theGrowCount;
    mTotalItems = 0;
    mItemSize = theItemSize;
}

void TodAllocator::Dispose() { FreeAll(); }

// 0x4438C0
void TodAllocator::Grow() {
    TOD_ASSERT(mGrowCount > 0);
    TOD_ASSERT(static_cast<size_t>(mItemSize) >= sizeof(void *));

    void *aBlock = TodMalloc(mGrowCount * mItemSize + sizeof(void *));
    *static_cast<void **>(aBlock) = mBlockList;
    mBlockList = aBlock;

    void *aFreeList = mFreeList;
    auto aItem = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(aBlock) + sizeof(void *));
    for (int i = 0; i < mGrowCount; i++) {
        *static_cast<void **>(aItem) = aFreeList;
        aFreeList = aItem;
        aItem = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(aItem) + mItemSize);
    }
    mFreeList = aFreeList;
}

bool TodAllocator::IsPointerFromAllocator(void *theItem) const {
    const size_t aBlockSize = mGrowCount * mItemSize; // 每次“Grow”的内存大小，即每个区块的内存大小
    for (void *aPtr = mBlockList; aPtr != nullptr; aPtr = *static_cast<void **>(aPtr)) {
        const uintptr_t aItemPtr = reinterpret_cast<uintptr_t>(theItem);
        // 区块的首个四字节为额外申请的、用于存储指向下一区块的指针的区域
        const uintptr_t aBlockPtr = reinterpret_cast<uintptr_t>(aPtr) + sizeof(void *);
        // 判断 theItem 是否位于当前区块内且指向某一项的区域的起始地址
        if (aItemPtr >= aBlockPtr && aItemPtr < aBlockPtr + aBlockSize && (aItemPtr - aBlockPtr) % mItemSize == 0)
            return true;
    }
    return false;
}

bool TodAllocator::IsPointerOnFreeList(const void *theItem) const {
    for (void *aPtr = mFreeList; aPtr != nullptr; aPtr = *static_cast<void **>(aPtr))
        if (theItem == aPtr) return true;
    return false;
}

void *TodAllocator::Alloc(int theItemSize) {
    (void)theItemSize;
    mTotalItems++;
    if (mFreeList == nullptr) Grow();

    const auto anItem = (void *)mFreeList;
    mFreeList = *static_cast<void **>(anItem);
    return anItem;
}

void *TodAllocator::Calloc(int theItemSize) {
    void *anItem = Alloc(theItemSize);
    memset(anItem, 0, theItemSize);
    return anItem;
}

void TodAllocator::Free(void *theItem, int theItemSize) {
    (void)theItemSize;
    mTotalItems--;
    TOD_ASSERT(IsPointerFromAllocator(theItem));
    TOD_ASSERT(!IsPointerOnFreeList(theItem));
    *static_cast<void **>(theItem) = mFreeList; // 将原可用区域头存入 [*theItem] 中
    mFreeList = theItem;                        // 将 theItem 设为新的可用区域头
}

void TodAllocator::FreeAll() {
    for (void *aBlock = mBlockList; aBlock != nullptr;) {
        void *aNext = *static_cast<void **>(aBlock);
        TodFree(aBlock);
        aBlock = aNext;
    }

    mBlockList = nullptr;
    mFreeList = nullptr;
    mTotalItems = 0;
}
