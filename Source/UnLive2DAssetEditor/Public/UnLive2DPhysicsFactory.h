#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "UnLive2DPhysicsFactory.generated.h"


UCLASS()
class UNLIVE2DASSETEDITOR_API UUnLive2DPhysicsFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, Category = UnLive2DMotionFactory)
	class UUnLive2D* TargetUnLive2D;


protected:
	// 判定支持的类
	virtual bool DoesSupportClass(UClass* Class) override;

	// 此工厂是否可以导入指定的文件。
	virtual bool FactoryCanImport(const FString& Filename) override;

	// 通过从文件名导入新对象来创建新对象。
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

};