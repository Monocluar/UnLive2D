#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Blueprint.h"
#include "UnLive2DMotion.h"
#include "UnLive2DAnimBlueprint.generated.h"

class UUnLive2DAnimBlueprintGeneratedClass;
class UUnLive2DMotion;
class URigVMController;

USTRUCT()
struct FUnLive2DAnimParentNodeAssetOverride
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY()
		UUnLive2DMotion* NewAsset;
	UPROPERTY()
		FGuid ParentNodeGuid;

	FUnLive2DAnimParentNodeAssetOverride(FGuid InGuid, UUnLive2DMotion* InNewAsset)
		: NewAsset(InNewAsset)
		, ParentNodeGuid(InGuid)
	{}

	FUnLive2DAnimParentNodeAssetOverride()
		: NewAsset(NULL)
	{}

	bool operator ==(const FUnLive2DAnimParentNodeAssetOverride& Other)
	{
		return ParentNodeGuid == Other.ParentNodeGuid;
	}
};

UCLASS(BlueprintType)
class UNLIVE2DASSET_API UUnLive2DAnimBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:

	UUnLive2DAnimBlueprintGeneratedClass* GetAnimBlueprintGeneratedClass() const;

	UUnLive2DAnimBlueprintGeneratedClass* GetUnLive2DAnimBlueprintClass() const;

public:
	// 重新编译虚拟机
	UFUNCTION(BlueprintCallable, Category = "UnLive2D Anim Blueprint")
		void RecompileVM();

public:
	// 目标UnLive2D数据资源
	UPROPERTY(AssetRegistrySearchable, EditAnywhere, AdvancedDisplay, Category=ClassOptions)
		UUnLive2D* TargetUnLive2D;

protected:

#if WITH_EDITOR

	virtual UClass* GetBlueprintClass() const override;

	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
	virtual bool IsValidForBytecodeOnlyRecompile() const override { return false; }
	virtual void LoadModulesRequiredForCompilation() override {};
	virtual void GetTypeActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// End of UBlueprint interface

#endif

	void CleanupBoneHierarchyDeprecated();
	void CreateMemberVariablesOnLoad();

public:

	UUnLive2DAnimBlueprint* GetPreviewAnimationBlueprint() const;

#if WITH_EDITORONLY_DATA
public:
	// 对包含父级中已被覆盖节点的资源的覆盖数组
	UPROPERTY()
		TArray<FUnLive2DAnimParentNodeAssetOverride> ParentAssetOverrides;
#endif

public:

	UPROPERTY(BlueprintReadOnly, transient, Category = "VM")
		URigVMController* Controller;

private:

	friend class FUnLive2DAnimBlueprintComilerContext;
};