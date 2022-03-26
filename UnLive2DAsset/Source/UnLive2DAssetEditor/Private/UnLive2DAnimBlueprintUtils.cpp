#include "UnLive2DAnimBlueprintUtils.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

FName FUnLive2DAnimBlueprintUtils::ValidateName(UBlueprint* InBlueprint, const FString& InName)
{
	FString Name = InName;
	if (Name.StartsWith(TEXT("UnLive2DRigUnit_")))
	{
		Name.RightChopInline(16, false);
	}

	TSharedPtr<FKismetNameValidator> NameValidator;
	NameValidator = MakeShareable(new FKismetNameValidator(InBlueprint));

	// Clean up BaseName to not contain any invalid characters, which will mean we can never find a legal name no matter how many numbers we add
	if (NameValidator->IsValid(Name) == EValidatorResult::ContainsInvalidCharacters)
	{
		for (TCHAR& TestChar : Name)
		{
			for (TCHAR BadChar : UE_BLUEPRINT_INVALID_NAME_CHARACTERS)
			{
				if (TestChar == BadChar)
				{
					TestChar = TEXT('_');
					break;
				}
			}
		}
	}

	if (UClass* ParentClass = InBlueprint->ParentClass)
	{
		FFieldVariant ExistingField = FindUFieldOrFProperty(ParentClass, *Name);
		if (ExistingField)
		{
			Name = FString::Printf(TEXT("%s_%d"), *Name, 0);
		}
	}

	int32 Count = 0;
	FString BaseName = Name;
	while (NameValidator->IsValid(Name) != EValidatorResult::Ok)
	{
		// Calculate the number of digits in the number, adding 2 (1 extra to correctly count digits, another to account for the '_' that will be added to the name
		int32 CountLength = Count > 0 ? (int32)log((double)Count) + 2 : 2;

		// If the length of the final string will be too long, cut off the end so we can fit the number
		if (CountLength + BaseName.Len() > NameValidator->GetMaximumNameLength())
		{
			BaseName.LeftInline(NameValidator->GetMaximumNameLength() - CountLength);
		}
		Name = FString::Printf(TEXT("%s_%d"), *BaseName, Count);
		Count++;
	}

	return *Name;
}

