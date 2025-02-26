
#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"

class SWindow;
class UUnLive2DImportUI;

class SUnLive2DOptionWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DOptionWindow)
		: _WidgetWindow()
		, _ImportUI(nullptr)
		, _FullPath()
		, _MaxWindowHeight(0.f)
		, _MaxWindowWidth(0.f)
		{}
		SLATE_ARGUMENT( TSharedPtr<SWindow>, WidgetWindow )
		SLATE_ARGUMENT( UUnLive2DImportUI*, ImportUI )
		SLATE_ARGUMENT( FName,  FullPath)
		SLATE_ARGUMENT(float, MaxWindowHeight)
		SLATE_ARGUMENT(float, MaxWindowWidth)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }

	// 是否可以导入
	bool ShouldImport() const
	{
		return bShouldImport;
	}

	SUnLive2DOptionWindow()
		: ImportUI(nullptr)
		, bShouldImport(false)
	{}

protected:

	// 导入
	FReply OnImport()
	{
		bShouldImport = true;
		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	// 取消
	FReply OnCancel()
	{
		bShouldImport = false;
		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	// 用户按ESC键取消
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			return OnCancel();
		}

		return FReply::Unhandled();
	}

	// 恢复默认设置
	FReply OnResetToDefaultClick() const; 

private:
	TSharedPtr<class IDetailsView> DetailsView;
	TWeakPtr<SWindow> WidgetWindow;
	UUnLive2DImportUI* ImportUI;

	bool bShouldImport;

};