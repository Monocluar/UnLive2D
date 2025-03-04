
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "EditorUnLive2DDisplayInfo.generated.h"

USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FEditorUnLive2DGroupParameterList
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	TArray<FName> ParameterArr;
};

//  Live2D 展示信息
UCLASS(Blueprintable, BlueprintType, Config = Editor, hidecategories = Object)
class UNLIVE2DASSET_API UEditorUnLive2DDisplayInfo : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	bool LoadLive2DDisplayInfo(const FString& FileContent);
#endif

public:
#if WITH_EDITORONLY_DATA
	// 参数ID对照名称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TMap<FName, FName> ParameterIDToName;

	// 组ID含有的参数ID数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TMap<FName, FEditorUnLive2DGroupParameterList> GroupIDToParameterArr;

	// 组ID对照名称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TMap<FName, FName> GroupIDToName;

	// Part ID对照名称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TMap<FName, FName> PartIDToName;
#endif
};