#include "UnLive2DBlueprintFactory.h"
#include "UObject/Interface.h"
#include "Misc/MessageDialog.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AssetData.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "Animation/UnLive2DAnimInstance.h"
#include "Animation/UnLive2DAnimBlueprintGeneratedClass.h"
#include "ClassViewerFilter.h"
#include "UnLive2D.h"
#include "ClassViewerModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

static bool CanCreateAnimBlueprint(const FAssetData& UnLive2DAsset, UClass const* ParentClass)
{
	if (UnLive2DAsset.IsValid() && ParentClass != nullptr)
	{
		if (UUnLive2DAnimBlueprintGeneratedClass const* GeneratedParent = Cast<const UUnLive2DAnimBlueprintGeneratedClass>(ParentClass))
		{
			if (UnLive2DAsset.GetExportTextName() != FAssetData(GeneratedParent->GetTargetUnLive2D()).GetExportTextName())
			{
				return false;
			}
		}
	}

	return true;
}

class SUnLive2DBlueprintCreateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DBlueprintCreateDialog) {}

	SLATE_END_ARGS()

public:
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs)
	{
		bOkClicked = false;
		ParentClass = UUnLive2DAnimInstance::StaticClass();

		ChildSlot
		[
			SNew(SBorder)
			.Visibility(EVisibility::Visible)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible)
				.WidthOverride(500.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Content()
						[
							SAssignNew(ParentClassContainer, SVerticalBox)
						]
					]
					
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Content()
						[
							SAssignNew(UnLive2DContainer, SVerticalBox)
						]
					]

					// Ok/Cancel buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(8.f)
					[
						SNew(SUniformGridPanel)
						.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
						.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
						.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
							.OnClicked(this, &SUnLive2DBlueprintCreateDialog::OkClicked)
							.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintOk", "OK"))
						]
						+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
							.OnClicked(this, &SUnLive2DBlueprintCreateDialog::CancelClicked)
							.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintCancel", "Cancel"))
						]
					]
				]
			]
		];

		MakeParentClassPicker();
		MakeUnLive2DPicker();
	}

	// Sets properties for the supplied UnLive2DBlueprintFactory
	bool ConfigureProperties(TWeakObjectPtr<UUnLive2DBlueprintFactory> InAnimBlueprintFactory)
	{
		AnimBlueprintFactory = InAnimBlueprintFactory;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("CreateAnimBlueprintOptions", "创建UnLive2D动画蓝图"))
			.ClientSize(FVector2D(400, 700))
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			[
				AsShared()
			];

		PickerWindow = Window;

		GEditor->EditorAddModalWindow(Window);
		AnimBlueprintFactory.Reset();

		return bOkClicked;
	}

private:
	class FUnLive2DAnimBlueprintParentFilter : public IClassViewerFilter
	{
	public:
		/** All children of these classes will be included unless filtered out by another setting. */
		TSet< const UClass* > AllowedChildrenOfClasses;
		const FAssetData& ShouldBeCompatibleWithUnLive2D;

	public:
		FUnLive2DAnimBlueprintParentFilter(const FAssetData& UnLive2D) : ShouldBeCompatibleWithUnLive2D(UnLive2D){}

	protected:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			if (InClass)
			{
				if (InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) == EFilterReturn::Failed) return false;

				return CanCreateAnimBlueprint(ShouldBeCompatibleWithUnLive2D, InClass);
			}

			return false;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			// If it appears on the allowed child-of classes list (or there is nothing on that list)
			return InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
		}
	};

	/** Handler for when a parent class is selected */
	void OnClassPicked(UClass* ChosenClass)
	{
		ParentClass = ChosenClass;
		MakeUnLive2DPicker();
	}

	/** Creates the combo menu for the parent class */
	void MakeParentClassPicker()
	{
		// Load the classviewer module to display a class picker
		FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

		// Fill in options
		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;

		TSharedPtr<FUnLive2DAnimBlueprintParentFilter> Filter = MakeShareable(new FUnLive2DAnimBlueprintParentFilter(TargetUnLive2D));
		Options.ClassFilter = Filter;

		// All child child classes of UUnLive2DAnimInstance are valid.
		Filter->AllowedChildrenOfClasses.Add(UUnLive2DAnimInstance::StaticClass());

		ParentClassContainer->ClearChildren();
		ParentClassContainer->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ParentClass", "Parent Class:"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			];

		ParentClassContainer->AddSlot()
			[
				ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SUnLive2DBlueprintCreateDialog::OnClassPicked))
			];
	}

	/** Creates the combo menu for the target UnLive2D */
	void MakeUnLive2DPicker()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		FAssetPickerConfig AssetPickerConfig;
		AssetPickerConfig.Filter.ClassNames.Add(UUnLive2D::StaticClass()->GetFName());
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DBlueprintCreateDialog::OnUnLive2DSelected);
		AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DBlueprintCreateDialog::FilterUnLive2DBasedOnParentClass);
		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
		AssetPickerConfig.InitialAssetSelection = TargetUnLive2D;

		UnLive2DContainer->ClearChildren();
		UnLive2DContainer->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetUnLive2D", "UnLive2D数据资源:"))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		];

		UnLive2DContainer->AddSlot()
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
	}

	bool FilterUnLive2DBasedOnParentClass(const FAssetData& AssetData)
	{
		return !CanCreateAnimBlueprint(AssetData, ParentClass.Get());
	}

	/** Handler for when a skeleton is selected */
	void OnUnLive2DSelected(const FAssetData& AssetData)
	{
		TargetUnLive2D = AssetData;
	}

	void CloseDialog(bool bWasPicked = false)
	{
		bOkClicked = bWasPicked;
		if (PickerWindow.IsValid())
		{
			PickerWindow.Pin()->RequestDestroyWindow();
		}
	}

	/** Handler for when ok is clicked */
	FReply OkClicked()
	{
		if (AnimBlueprintFactory.IsValid())
		{
			AnimBlueprintFactory->BlueprintType = BPTYPE_Normal;
			AnimBlueprintFactory->ParentClass = ParentClass.Get();
			AnimBlueprintFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2D.GetAsset());
		}

		if ( !TargetUnLive2D.IsValid())
		{
			// if TargetSkeleton is not valid
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidUnLive2D", "必须为动画蓝图指定有效的Live2D数据."));
			return FReply::Handled();
		}

		if ( !CanCreateAnimBlueprint(TargetUnLive2D, ParentClass.Get()))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedCompatibleUnLive2D", "选定的骨架必须与选定的父类兼容."));
			return FReply::Handled();
		}

		CloseDialog(true);

		return FReply::Handled();
	}

	/** Handler for when cancel is clicked */
	FReply CancelClicked()
	{
		CloseDialog();
		return FReply::Handled();
	}

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CloseDialog();
			return FReply::Handled();
		}
		return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

private:
	/** The factory for which we are setting up properties */
	TWeakObjectPtr<UUnLive2DBlueprintFactory> AnimBlueprintFactory;

	/** A pointer to the window that is asking the user to select a parent class */
	TWeakPtr<SWindow> PickerWindow;

	/** The container for the Parent Class picker */
	TSharedPtr<SVerticalBox> ParentClassContainer;

	/** The container for the target skeleton picker*/
	TSharedPtr<SVerticalBox> UnLive2DContainer;

	/** The selected class */
	TWeakObjectPtr<UClass> ParentClass;

	/** True if Ok was clicked */
	bool bOkClicked;

	/** The selected UnLive2D */
	FAssetData TargetUnLive2D;
};

UUnLive2DBlueprintFactory::UUnLive2DBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UUnLive2DAnimBlueprint::StaticClass();
	ParentClass = UUnLive2DAnimInstance::StaticClass();
}

bool UUnLive2DBlueprintFactory::ConfigureProperties()
{
	TSharedRef<SUnLive2DBlueprintCreateDialog> Dialog = SNew(SUnLive2DBlueprintCreateDialog);
	return Dialog->ConfigureProperties(this);
}

UObject* UUnLive2DBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a UnLive2D Anim Blueprint, then create and init one
	check(Class->IsChildOf(UUnLive2DAnimBlueprint::StaticClass()));

	// If they selected an interface, we dont need a target UnLive2D
	if (BlueprintType != BPTYPE_Interface && TargetUnLive2D == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidSkeleton", "必须为动画蓝图指定一个UnLive2D数据源"));
		return nullptr;
	}

	if (BlueprintType != BPTYPE_Interface && (ParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass) || !ParentClass->IsChildOf(UUnLive2DAnimInstance::StaticClass()))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateAnimBlueprint", "无法基于类创建动画蓝图'{ClassName}'."), Args));
		return nullptr;
	}
	else
	{
		UClass* ClassToUse = ParentClass.Get();
		UUnLive2DAnimBlueprint* NewBP = CastChecked<UUnLive2DAnimBlueprint>(FKismetEditorUtilities::CreateBlueprint(ClassToUse, InParent, Name, BlueprintType, UUnLive2DAnimBlueprint::StaticClass(), UUnLive2DAnimBlueprintGeneratedClass::StaticClass(), CallingContext));

		// Inherit any existing overrides in parent class
		if (NewBP->ParentAssetOverrides.Num() > 0)
		{
			// We've inherited some overrides from the parent graph and need to recompile the blueprint.
			FKismetEditorUtilities::CompileBlueprint(NewBP);
		}

		NewBP->TargetUnLive2D = TargetUnLive2D;

		// Because the BP itself didn't have the skeleton set when the initial compile occured, it's not set on the generated classes either
		if (UUnLive2DAnimBlueprintGeneratedClass* TypedNewClass = Cast<UUnLive2DAnimBlueprintGeneratedClass>(NewBP->GeneratedClass))
		{
			TypedNewClass->TargetUnLive2D = TargetUnLive2D;
		}
		if (UUnLive2DAnimBlueprintGeneratedClass* TypedNewClass_SKEL = Cast<UUnLive2DAnimBlueprintGeneratedClass>(NewBP->SkeletonGeneratedClass))
		{
			TypedNewClass_SKEL->TargetUnLive2D = TargetUnLive2D;
		}

		return NewBP;
	}
}

UObject* UUnLive2DBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE