#include "Editor/EditorUnLive2DDisplayInfo.h"

#if WITH_EDITOR
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#endif


#if WITH_EDITOR
bool UEditorUnLive2DDisplayInfo::LoadLive2DDisplayInfo(const FString& FileContent)
{
	const TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(FileContent);
	TSharedPtr<FJsonObject> JsonObject;

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid()) return false;

	ParameterIDToName.Empty();
	GroupIDToParameterArr.Empty();
	GroupIDToName.Empty();
	PartIDToName.Empty();
	int32 Version;
	checkf(JsonObject->TryGetNumberField(TEXT("Version"), Version), TEXT("Failed to get Version field."));

	const TArray< TSharedPtr<FJsonValue> >* ParametersArrayObject;
	checkf(JsonObject->TryGetArrayField(TEXT("Parameters"), ParametersArrayObject), TEXT("Failed to get Parameters field."));
	for (const TSharedPtr<FJsonValue>& ParameterValue : *ParametersArrayObject)
	{
		const TSharedPtr<FJsonObject>& ParameterObject = ParameterValue->AsObject();
		FString GroupId;
		FString ParametersID;
		checkf(ParameterObject->TryGetStringField(TEXT("GroupId"), GroupId), TEXT("Failed to get GroupId field."));
		checkf(ParameterObject->TryGetStringField(TEXT("Id"), ParametersID), TEXT("Failed to get Id field."));

		FEditorUnLive2DGroupParameterList& ParameterArr = GroupIDToParameterArr.FindOrAdd(*GroupId);
		checkf(!ParameterArr.ParameterArr.Contains(*ParametersID), TEXT("Parameters contain duplicates Id"));
		ParameterArr.ParameterArr.Add(*ParametersID);

		FString NameString;

		checkf(ParameterObject->TryGetStringField(TEXT("Name"), NameString), TEXT("Failed to get Name field."));
		ParameterIDToName.FindOrAdd(*ParametersID) = *NameString;

	}

	const TArray< TSharedPtr<FJsonValue> >* ParameterGroupsArrayObject;
	checkf(JsonObject->TryGetArrayField(TEXT("ParameterGroups"), ParameterGroupsArrayObject), TEXT("Failed to get ParameterGroups field."));
	for (const TSharedPtr<FJsonValue>& ParameterGroupValue : *ParameterGroupsArrayObject)
	{
		const TSharedPtr<FJsonObject>& ParameterGroupObject = ParameterGroupValue->AsObject();

		FString NameString;
		FString GroupsID;
		checkf(ParameterGroupObject->TryGetStringField(TEXT("Id"), GroupsID), TEXT("Failed to get Id field."));
		checkf(ParameterGroupObject->TryGetStringField(TEXT("Name"), NameString), TEXT("Failed to get Name field."));
		GroupIDToName.FindOrAdd(*GroupsID) = *NameString;
	}

	const TArray< TSharedPtr<FJsonValue> >* PartsArrayObject;
	checkf(JsonObject->TryGetArrayField(TEXT("Parts"), PartsArrayObject), TEXT("Failed to get Parts field."));
	for (const TSharedPtr<FJsonValue>& PartValue : *PartsArrayObject)
	{
		const TSharedPtr<FJsonObject>& PartObject = PartValue->AsObject();
		FString PartId;
		checkf(PartObject->TryGetStringField(TEXT("Id"), PartId), TEXT("Failed to get Id field."));

		FString NameString;
		checkf(PartObject->TryGetStringField(TEXT("Name"), NameString), TEXT("Failed to get Name field."));

		PartIDToName.FindOrAdd(*PartId) = *NameString;
	}
	return true;
}
#endif