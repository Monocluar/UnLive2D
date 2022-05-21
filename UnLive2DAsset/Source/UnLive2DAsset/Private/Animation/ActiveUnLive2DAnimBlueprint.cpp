#include "Animation/ActiveUnLive2DAnimBlueprint.h"

FActiveUnLive2DAnimBlueprint::FActiveUnLive2DAnimBlueprint()
{

}

FActiveUnLive2DAnimBlueprint::~FActiveUnLive2DAnimBlueprint()
{

}

uint32 FActiveUnLive2DAnimBlueprint::GetObjectId() const
{
	if (!UnLive2DAnimBlueprint.IsValid()) return INDEX_NONE;

	return UnLive2DAnimBlueprint->GetUniqueID();
}

