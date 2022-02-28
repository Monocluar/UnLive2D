#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "UnLive2DFactory.generated.h"

UCLASS()
class UNLIVE2DASSETEDITOR_API UUnLive2DFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:

	// UFactory interface
	
	// 判定支持的类
	virtual bool DoesSupportClass(UClass* Class) override;

	/*// 按类在编辑器创建新对象
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;*/

	// 此工厂是否可以导入指定的文件。
	virtual bool FactoryCanImport(const FString& Filename) override;

	// 通过从文件名导入新对象来创建新对象。
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;


	virtual void PostInitProperties() override;
	
	// End of UFactory interface

	/** 从包和objectname和类创建新资源 */
	static UObject* CreateAssetOfClass(UClass* AssetClass, FString ParentPackageName, FString ObjectName, bool bAllowReplace = false);

	/* 模板化函数，用于创建具有给定包和名称的资产 */
	template< class T = UObject>
	static T* CreateAsset(FString ParentPackageName, FString ObjectName, bool bAllowReplace = false)
	{
		return (T*)CreateAssetOfClass(T::StaticClass(), ParentPackageName, ObjectName, bAllowReplace);
	}

public:
	UPROPERTY()
		class UUnLive2DImportUI* ImportUI;
};