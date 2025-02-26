#include "CubismFrameWorkAllocator.h"
#include "HAL/UnrealMemory.h"

using namespace Csm;

void* CubismFrameWorkAllocator::Allocate(const csmSizeType Size)
{
    return FMemory::Malloc(Size);
}

void CubismFrameWorkAllocator::Deallocate(void* Memory)
{
    FMemory::Free(Memory);
}

void* CubismFrameWorkAllocator::AllocateAligned(const csmSizeType Size, const csmUint32 Alignment)
{
    size_t Offset, Shift, AlignedAddress;
    void* Allocation;
    void** Preamble;

    Offset = Alignment - 1 + sizeof(void*);

    Allocation = Allocate(Size + static_cast<csmUint32>(Offset));

    AlignedAddress = reinterpret_cast<size_t>(Allocation) + sizeof(void*);

    Shift = AlignedAddress % Alignment;

    if (Shift)
    {
        AlignedAddress += (Alignment - Shift);
    }

    Preamble = reinterpret_cast<void**>(AlignedAddress);
    Preamble[-1] = Allocation;

    return reinterpret_cast<void*>(AlignedAddress);
}

void CubismFrameWorkAllocator::DeallocateAligned(void* AlignedMemory)
{
    void** Preamble;

    Preamble = static_cast<void**>(AlignedMemory);

    Deallocate(Preamble[-1]);
}
