
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "Engine/Blueprint.h"
#include "Factories/Factory.h"
#include "Engine/Blueprint.h"

#include "UnLive2DBlueprintFactory.generated.h"

class UUnLive2DAnimInstance;

UCLASS(MinimalAPI, hideCategories=Object)
class UUnLive2DBlueprintFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, Category = UnLive2DBlueprintFactory)
		class UUnLive2D* TargetUnLive2D;


protected:
	// UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};