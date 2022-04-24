#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Base.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphSchema.h"
#include "EdGraph/EdGraphNode.h"

UUnLive2DAnimBlueprintGraphNode_Base::UUnLive2DAnimBlueprintGraphNode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UEdGraphPin* UUnLive2DAnimBlueprintGraphNode_Base::GetOutputPin()
{
	UEdGraphPin* OutputPin = NULL;

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction != EGPD_Output) continue;

		OutputPin = Pins[PinIndex];
	}

	return OutputPin;
}

void UUnLive2DAnimBlueprintGraphNode_Base::GetInputPins(TArray<class UEdGraphPin*>& OutInputPins)
{
	OutInputPins.Empty();

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction != EGPD_Input) continue;

		OutInputPins.Add(Pins[PinIndex]);
	}
}

UEdGraphPin* UUnLive2DAnimBlueprintGraphNode_Base::GetGetInputPin(int32 InputIndex)
{
	check(InputIndex >= 0 && InputIndex < GetInputCount());

	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			if (InputIndex == FoundInputs)
			{
				return Pins[PinIndex];
			}
			else
			{
				FoundInputs++;
			}
		}
	}

	return nullptr;
}

int32 UUnLive2DAnimBlueprintGraphNode_Base::GetInputCount() const
{
	int32 InputCount = 0;
	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction != EGPD_Input) continue;

		InputCount++;
	}

	return InputCount;
}

void UUnLive2DAnimBlueprintGraphNode_Base::InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList)
{

}

void UUnLive2DAnimBlueprintGraphNode_Base::AllocateDefaultPins()
{
	check(Pins.Num() == 0);

	CreateInputPins();

	if (!IsRootNode())
	{
		CreatePin(EGPD_Output, TEXT("UnLive2DAnimBlueprintNode"), TEXT("Output"));
	}
}

void UUnLive2DAnimBlueprintGraphNode_Base::ReconstructNode()
{
	// Break any links to 'orphan' pins
	for (int32 PinIndex = 0; PinIndex < Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = Pins[PinIndex];
		TArray<UEdGraphPin*>& LinkedToRef = Pin->LinkedTo; // 我们链接到的一组PIN
		for (int32 LinkIdx = 0; LinkIdx < LinkedToRef.Num(); LinkIdx++)
		{
			UEdGraphPin* OtherPin = LinkedToRef[LinkIdx];
			// 如果我们链接到一个其所有者不知道的pin，断开该链接
			if (!OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	// 存储旧的输入和输出引脚
	TArray<UEdGraphPin*> OldInputPins;
	GetInputPins(OldInputPins);
	UEdGraphPin* OldOutputPin = GetOutputPin();

	// 将现有管脚移动到保存的阵列
	TArray<UEdGraphPin*> OldPins(Pins);
	Pins.Reset();

	// 重新创建新的PIN
	AllocateDefaultPins();

	// 获取新的输入输出引脚
	TArray<UEdGraphPin*> NewInputPins;
	GetInputPins(NewInputPins);
	UEdGraphPin* NewOutputPin = GetOutputPin();

	for (int32 PinIndex = 0; PinIndex < OldInputPins.Num(); PinIndex++)
	{
		if (PinIndex < NewInputPins.Num())
		{
			NewInputPins[PinIndex]->MovePersistentDataFromOldPin(*OldInputPins[PinIndex]); // 移动引脚数据
		}
	}

	if (OldOutputPin)
	{
		NewOutputPin->MovePersistentDataFromOldPin(*OldOutputPin);
	}

	// 清理老引脚数据
	for (UEdGraphPin* OldPin : OldPins)
	{
		OldPin->Modify();
		UEdGraphNode::DestroyPin(OldPin);
	}
}

void UUnLive2DAnimBlueprintGraphNode_Base::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (FromPin == NULL) return;

	const UUnLive2DAnimBlueprintGraphSchema* Schema = CastChecked<UUnLive2DAnimBlueprintGraphSchema>(GetSchema());

	TSet<UEdGraphNode*> NodeList;

	// 自动连接新节点上从拖动的管脚到第一个兼容管脚
	for (int32 i = 0; i < Pins.Num(); i++)
	{
		UEdGraphPin* Pin = Pins[i];
		check(Pin);
		FPinConnectionResponse Response = Schema->CanCreateConnection(FromPin, Pin);
		if (ECanCreateConnectionResponse::CONNECT_RESPONSE_MAKE == Response.Response) 
		{
			if (Schema->TryCreateConnection(FromPin, Pin))
			{
				NodeList.Add(FromPin->GetOwningNode());
				NodeList.Add(this);
			}
			break;
		}
		else if (ECanCreateConnectionResponse::CONNECT_RESPONSE_BREAK_OTHERS_A == Response.Response)
		{
			InsertNewNode(FromPin, Pin, NodeList);
			break;
		}
	}

	// 向所有接收到新pin连接的节点发送通知
	for (auto It = NodeList.CreateConstIterator(); It; ++It)
	{
		UEdGraphNode* Node = (*It);
		Node->NodeConnectionListChanged();
	}
}

bool UUnLive2DAnimBlueprintGraphNode_Base::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema->IsA(UUnLive2DAnimBlueprintGraphSchema::StaticClass());
}

FString UUnLive2DAnimBlueprintGraphNode_Base::GetDocumentationLink() const
{
	return TEXT("Shared/GraphNodes/UnLive2DAnimBlueprint");
}

