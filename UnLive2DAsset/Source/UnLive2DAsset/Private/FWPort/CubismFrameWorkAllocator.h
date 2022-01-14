#pragma once

#include <CubismFramework.hpp>
#include <ICubismAllocator.hpp>

class CubismFrameWorkAllocator : public Csm::ICubismAllocator
{
public:
    virtual void* Allocate(const Csm::csmSizeType Size) override;

    virtual void Deallocate(void* Memory) override;

    virtual void* AllocateAligned(const Csm::csmSizeType Size, const Csm::csmUint32 Alignment) override;

    virtual void DeallocateAligned(void* AlignedMemory) override;
};