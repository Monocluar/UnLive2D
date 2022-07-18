
#include "UnLive2DRawModel.h"

#include "CubismModelSettingJson.hpp"
#include "CubismFramework.hpp"
#include "CubismDefaultParameterId.hpp"
#include "Id/CubismIdManager.hpp"
#include "Utils/CubismString.hpp"
#include "Motion/CubismMotion.hpp"

#include "UnLive2DAssetModule.h"
#include "CubismConfig.h"
#include "Animation/UnLive2DMotion.h"
#include "Misc/FileHelper.h"
#include "Animation/UnLive2DExpression.h"
#include "Live2DCubismCore.hpp"

// 动作优先度配置
const csmInt32 PriorityNone = 0;
const csmInt32 PriorityIdle = 1;
const csmInt32 PriorityNormal = 2;
const csmInt32 PriorityForce = 3;

// <匹配Live2D Josn Motions组的名字
const csmChar* MotionGroupIdle = "Idle";
const csmChar* MotionGroupTapBody = "TapBody";

// <匹配Live2D Josn HitAreas组的名字
const csmChar* HitAreaNameHead = "Head";
const csmChar* HitAreaNameBody = "Body";

TWeakObjectPtr<UUnLive2DMotion> CurrentPlayMotion = nullptr;

void OnMotionEnd(ACubismMotion* MotionData)
{
	UE_LOG(LogUnLive2D, Log, TEXT("播放动画完成"));
	if (CurrentPlayMotion.IsValid())
	{
		CurrentPlayMotion->OnPlayAnimEnd();
	}
}

FUnLive2DRawModel::FUnLive2DRawModel(class UUnLive2D* Owner)
	: Csm::CubismUserModel()
	, OwnerLive2D(Owner)
{
#if WITH_EDITOR
	UserTimeSeconds = 0;
#endif

	ID_ParamAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleX);
	ID_ParamAngleY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleY);
	ID_ParamAngleZ = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleZ);
	ID_ParamBodyAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamBodyAngleX);
	ID_ParamEyeBallX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallX);
	ID_ParamEyeBallY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallY);

	Live2DModelSetting = nullptr;
	PhysicsData = nullptr;
}

FUnLive2DRawModel::~FUnLive2DRawModel()
{
    ReleaseMotions();
    ReleaseExpressions();

    Live2DModelSetting.Reset();
	PhysicsData.Reset();
}

bool FUnLive2DRawModel::LoadAsset(const FUnLive2DLoadData& InData)
{
	if (InData.Live2DModelData.Num() == 0) 
	{
		UE_LOG(LogUnLive2D, Error, TEXT("FUnLive2DRawModel::LoadAsset: Read failed!"));
		return false;
	}

	// 读取Live2D 角色Josn数据
	Live2DModelSetting = MakeShared<CubismModelSettingJson>(InData.Live2DModelData.GetData(), InData.Live2DModelData.Num());

	// 立体模块
	if (InData.Live2DCubismData.Num() > 0)
	{
		LoadModel(InData.Live2DCubismData.GetData(), InData.Live2DCubismData.Num());

		UE_LOG(LogUnLive2D, Log, TEXT("FUnLive2DRawModel::LoadAsset: %f, %f"), _model->GetCanvasWidth(), _model->GetCanvasHeight());
	}
	else
	{
		UE_LOG(LogUnLive2D, Error, TEXT("FUnLive2DRawModel::LoadAsset:没有找到Live2DCubismData数据"));
		return false;
	}

	// 物理模块
	if (InData.Live2DPhysicsData.Num() > 0)
	{
		LoadPhysics(InData.Live2DPhysicsData.GetData(), InData.Live2DPhysicsData.Num());

		if (_physics)
		{
			PhysicsData = MakeShared<CubismPhysics::Options>();
		}
	}

	// 姿态模块
	if (InData.Live2DPoseData.Num() > 0)
	{
		LoadPose(InData.Live2DPoseData.GetData(), InData.Live2DPoseData.Num());
	}

	// 自动眨眼模块
	if (Live2DModelSetting->GetEyeBlinkParameterCount() > 0)
	{
		_eyeBlink = CubismEyeBlink::Create(Live2DModelSetting.Get());
	}

	// 眼睛链接
	{
		csmInt32 EyeBlinkIdCount = Live2DModelSetting->GetEyeBlinkParameterCount();
		for (csmInt32 i = 0; i < EyeBlinkIdCount; ++i)
		{
			EyeBlinkIds.PushBack(Live2DModelSetting->GetEyeBlinkParameterId(i));
		}
	}

	// 呼吸系统
	{
		_breath = CubismBreath::Create();

		csmVector<CubismBreath::BreathParameterData> BreathParameters;

		BreathParameters.PushBack(CubismBreath::BreathParameterData(ID_ParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
		BreathParameters.PushBack(CubismBreath::BreathParameterData(ID_ParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
		BreathParameters.PushBack(CubismBreath::BreathParameterData(ID_ParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
		BreathParameters.PushBack(CubismBreath::BreathParameterData(ID_ParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
		BreathParameters.PushBack(CubismBreath::BreathParameterData(CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

		_breath->SetParameters(BreathParameters);
	}

	// 用户数据
	if (InData.Live2DUserDataData.Num() > 0)
	{
		LoadUserData(InData.Live2DUserDataData.GetData(), InData.Live2DUserDataData.Num());
	}

	// 口型系统
	{
		csmInt32 LipSyncIdCount = Live2DModelSetting->GetLipSyncParameterCount();
		for (csmInt32 i = 0; i < LipSyncIdCount; ++i)
		{
			LipSyncIds.PushBack(Live2DModelSetting->GetLipSyncParameterId(i));
		}
	}

	// 层级设置
	csmMap<csmString, csmFloat32> layout;
	Live2DModelSetting->GetLayoutMap(layout);
	_modelMatrix->SetupFromLayout(layout);

	if (_model)
	{
		_model->SaveParameters();
	}

	_motionManager->StopAllMotions();

	return true;
}

void FUnLive2DRawModel::OnUpDate(float InDeltaTime)
{
#if WITH_EDITOR
	UserTimeSeconds += InDeltaTime;
#endif

	_dragManager->Update(InDeltaTime);
	_dragX = _dragManager->GetX();
	_dragY = _dragManager->GetY();

	// 是否通过动作更新参数
	csmBool MotionUpdated = false;

	if (_model == nullptr)
	{
		return;
	}

	// 装入上次保存的状态
	_model->LoadParameters();

	// 动作是否完成
	if (_motionManager->IsFinished())
	{
		OnMotionPlayEnd.ExecuteIfBound();

		// 没有动作的时候，从Idle动作组里面随机选择一个播放
		//StartRandomMotion(MotionGroupIdle, PriorityIdle);
	}
	else
	{
		MotionUpdated = _motionManager->UpdateMotion(_model, InDeltaTime); // 更新动作
	}

	_model->SaveParameters(); // 保存状态

	// 眨眼
	if (!MotionUpdated)
	{
		if (_eyeBlink != NULL)
		{
			// 没有更新主动作时
			_eyeBlink->UpdateParameters(_model, InDeltaTime); // 眨眼
		}
	}

	if (_expressionManager != NULL)
	{
		_expressionManager->UpdateMotion(_model, InDeltaTime); // 通过表情更新参数（相对变化）
	}

	//通过拖拽改变
	//通过拖拽调整脸部朝向
	/*_model->AddParameterValue(ID_ParamAngleX, _dragX * 30); // 加-30到30的值
	_model->AddParameterValue(ID_ParamAngleY, _dragY * 30);
	_model->AddParameterValue(ID_ParamAngleZ, _dragX * _dragY * -30);

	//通过拖曳调整身体方向
	_model->AddParameterValue(ID_ParamBodyAngleX, _dragX * 10); // 加-10到10的值

	//通过拖曳调整眼睛方向
	_model->AddParameterValue(ID_ParamEyeBallX, _dragX); // 从-1加1的值
	_model->AddParameterValue(ID_ParamEyeBallY, _dragY);*/

	// 呼吸
	if (_breath != NULL)
	{
		_breath->UpdateParameters(_model, InDeltaTime);
	}

	// 物理运算设置
	if (_physics != NULL)
	{
		_physics->Evaluate(_model, InDeltaTime);
	}

	// 唇型设置
	if (_lipSync)
	{
		csmFloat32 value = 0; // 实时进行唇型同步时，从系统取得音量，在0～1的范围内输入值。

		for (csmUint32 i = 0; i < LipSyncIds.GetSize(); ++i)
		{
			_model->AddParameterValue(LipSyncIds[i], value, 0.8f);
		}
	}

	// 姿势设置
	if (_pose != NULL)
	{
		_pose->UpdateParameters(_model, InDeltaTime);
	}

	_model->Update();
}

#if WITH_EDITOR
FUnLive2DLoadData FUnLive2DRawModel::LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& LoadTexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData)
{
	FUnLive2DLoadData LoadData;

	LoadTexturePaths.Empty();
	LoadMotionData.Empty();

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *InPath)) return LoadData;

	FString FileName;
	FString FileExtentsion;
	FString FileHomeDir;
	FPaths::Split(InPath, FileHomeDir, FileName, FileExtentsion);
	LoadData.Live2DModelData = FileData;
	CubismModelSettingJson* JsonData = new CubismModelSettingJson(FileData.GetData(), FileData.Num());

	// 立体模块
	if (strcmp(JsonData->GetModelFileName(), "") != 0)
	{
		csmString Path = JsonData->GetModelFileName();

		FString CubismModelPath = FileHomeDir / UTF8_TO_TCHAR(Path.GetRawString());

		TArray<uint8> CubismModelFile;
		const bool ReadSuc = FFileHelper::LoadFileToArray(CubismModelFile, *CubismModelPath);
		if (ReadSuc)
		{
			LoadData.Live2DCubismData = CubismModelFile;
		}
	}

	// 物理模块
	if (strcmp(JsonData->GetPhysicsFileName(), "") != 0)
	{
		csmString path = JsonData->GetPhysicsFileName();

		FString PhysicsPath = FileHomeDir / UTF8_TO_TCHAR(path.GetRawString());

		TArray<uint8> ModelFile;
		const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *PhysicsPath);
		if (ReadSuc)
		{
			LoadData.Live2DPhysicsData = ModelFile;
		}
	}

	// 姿态模块
	if (strcmp(JsonData->GetPoseFileName(), "") != 0)
	{
		csmString Path = JsonData->GetPoseFileName();
		FString TempReadPath = FileHomeDir / UTF8_TO_TCHAR(Path.GetRawString());

		TArray<uint8> ModelFile;
		const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *TempReadPath);

		if (ReadSuc)
		{
			LoadData.Live2DPoseData = ModelFile;
		}
	}

	// 用户模块
	if (strcmp(JsonData->GetUserDataFile(), "") != 0)
	{
		csmString Path = JsonData->GetUserDataFile();
		FString TempReadPath = FileHomeDir / UTF8_TO_TCHAR(Path.GetRawString());

		TArray<uint8> ModelFile;
		const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *TempReadPath);

		if (ReadSuc)
		{
			LoadData.Live2DUserDataData = ModelFile;
		}
	}

	csmInt32 td_TextureNum = JsonData->GetTextureCount();
	// 图片数据读取
	for (csmInt32 ModelTextureNumber = 0; ModelTextureNumber < td_TextureNum; ModelTextureNumber++)
	{
		if (strcmp(JsonData->GetTextureFileName(ModelTextureNumber), "") == 0) continue;

		Csm::csmString TexturePath = JsonData->GetTextureFileName(ModelTextureNumber);
		FString TempReadPath = FileHomeDir / UTF8_TO_TCHAR(TexturePath.GetRawString());

		if (TempReadPath.IsEmpty()) continue;

		/*TArray<uint8> ModelFile;
		const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *TempReadPath);
		if (ReadSuc)
		{
			LoadData.Live2DTexture2DData.Add(FUnLiveByteData(ModelFile));
		}*/

		LoadTexturePaths.Add(TempReadPath);
	}

	// 动作组数据读取
	// 动画系统
	for (csmInt32 i = 0; i < JsonData->GetMotionGroupCount(); i++)
	{
		const csmChar* Group = JsonData->GetMotionGroupName(i);
		if (Group == nullptr) continue;

		const csmInt32 Count = JsonData->GetMotionCount(Group);

		for (csmInt32 j = 0; j < Count; j++)
		{
			csmString Path = JsonData->GetMotionFileName(Group, j);

			FString TempReadPath = FileHomeDir / UTF8_TO_TCHAR(Path.GetRawString());

			FString PathFileName = FPaths::GetBaseFilename(TempReadPath);

			TArray<uint8> ModelFile;
			const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *TempReadPath);

			if (!ReadSuc) continue;

			FUnLive2DMotionData MotionData;

			MotionData.FadeInTime = JsonData->GetMotionFadeInTimeValue(Group, j);
			MotionData.FadeOutTime = JsonData->GetMotionFadeOutTimeValue(Group, j);
			MotionData.MotionByteData = ModelFile;

			UEnum* GroupType = StaticEnum<EUnLive2DMotionGroup>();
			MotionData.MotionGroupType = (EUnLive2DMotionGroup)GroupType->GetIndexByName(Group);

			MotionData.MotionCount = j;
			MotionData.PathName = PathFileName.LeftChop(8);
			LoadMotionData.Add(MotionData);

			//LoadData.Live2DMotionData.Add(Name.GetRawString(),FUnLiveByteData(ModelFile));
		}

	}

	// 表情模块
	if (JsonData->GetExpressionCount() > 0)
	{
		const csmInt32 Count = JsonData->GetExpressionCount();
		for (csmInt32 i = 0; i < Count; i++)
		{
			csmString Name = JsonData->GetExpressionName(i);
			csmString Path = JsonData->GetExpressionFileName(i);

			FString ExpressionPath = FileHomeDir / UTF8_TO_TCHAR(Path.GetRawString());

			TArray<uint8> ModelFile;
			const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *ExpressionPath);
			if (ReadSuc)
			{
				LoadExpressionData.Add(FPaths::GetBaseFilename(ExpressionPath).LeftChop(5), FUnLiveByteData(ModelFile));
			}
		}
	}

	delete JsonData;

	return LoadData;
}

#endif

void FUnLive2DRawModel::ReleaseMotions()
{
	for (TMap<FName, Csm::ACubismMotion*>::TConstIterator Iterator(Live2DMotions); Iterator; ++Iterator)
	{
		Csm::ACubismMotion* MotionPtr = Iterator.Value();
		if (MotionPtr == nullptr) continue;

		ACubismMotion::Delete(MotionPtr);
	}

    Live2DMotions.Empty();
}

void FUnLive2DRawModel::ReleaseExpressions()
{
	for (TMap<uint32, Csm::ACubismMotion*>::TConstIterator Iterator(Live2DExpressions); Iterator; ++Iterator)
	{
		Csm::ACubismMotion* ExpressionsPtr = Iterator.Value();
		if (ExpressionsPtr == nullptr) continue;

		ACubismMotion::Delete(ExpressionsPtr);
	}

    Live2DExpressions.Empty();
}

bool FUnLive2DRawModel::HitTest(const Csm::csmChar* HitAreaName, const FVector2D& InPos)
{
	// 透明不做判定
	if (_opacity < 1)
	{
		return false;
	}

	if (!Live2DModelSetting.IsValid())
	{
		return false;
	}

	const csmInt32 Count = Live2DModelSetting->GetHitAreasCount();
	for (csmInt32 i = 0; i < Count; i++)
	{
		if (strcmp(Live2DModelSetting->GetHitAreaName(i), HitAreaName) == 0)
		{
			const CubismIdHandle drawID = Live2DModelSetting->GetHitAreaId(i);
			return IsHit(drawID, InPos.X, InPos.Y);
		}
	}

	return false;

}

const char** FUnLive2DRawModel::GetLive2DModelParameterIds()
{
	return  Live2D::Cubism::Core::csmGetParameterIds(GetModel()->GetModel());
}

const float* FUnLive2DRawModel::GetLive2DModelParameterValues()
{
	return  Live2D::Cubism::Core::csmGetParameterValues(GetModel()->GetModel());
}

bool FUnLive2DRawModel::OnTapMotion(const FVector2D& InTapMotion)
{
	if (HitTest(HitAreaNameHead, InTapMotion))
	{
		SetRandomExpression();
		return true;
	}
	else if (HitTest(HitAreaNameBody, InTapMotion))
	{
		StartRandomMotion(MotionGroupTapBody, PriorityNormal);
		return true;
	}

	return false;
}

void FUnLive2DRawModel::SetDragPos(const FVector2D& InDragMotion)
{

	/*// 鼠标拖拽刷新
	_dragX = _dragManager->GetX();
	_dragY = _dragManager->GetY();*/

	if (_dragManager == nullptr) return;

	_dragManager->Set(InDragMotion.X,InDragMotion.Y);

}

void FUnLive2DRawModel::SetPhysicsGravity(const FVector2D& InGravity)
{
	if (PhysicsData.IsValid()) return;

	PhysicsData->Gravity = CubismVector2(InGravity.X, InGravity.Y);

	_physics->SetOptions(*PhysicsData);
}

void FUnLive2DRawModel::SetPhysicsWind(const FVector2D& InWind)
{
	if (PhysicsData.IsValid()) return;

	PhysicsData->Wind = CubismVector2(InWind.X, InWind.Y);

	_physics->SetOptions(*PhysicsData);
}

Csm::CubismMotionQueueEntryHandle FUnLive2DRawModel::StartMotion(const Csm::csmChar* Group, Csm::csmInt32 No, Csm::csmInt32 Priority)
{
	if (Priority == PriorityForce)
	{
		_motionManager->SetReservePriority(Priority);
	}
	else if (!_motionManager->ReserveMotion(Priority))
	{
		//UE_LOG(LogUnLive2D, Log, TEXT("FUnLive2DRawModel::StartMotion: Can't start motion."));
		return InvalidMotionQueueEntryHandleValue;
	}

	//const csmString FileName = Live2DModelSetting->GetMotionFileName(Group, No);

	csmString Name = Utils::CubismString::GetFormatedString("%s_%d", Group, No);

	Csm::ACubismMotion* FindPtr = Live2DMotions.FindOrAdd(Name.GetRawString());


	UE_LOG(LogUnLive2D, Log, TEXT("FUnLive2DRawModel::StartMotion: Start [%s_%d]"), UTF8_TO_TCHAR(Group), No);

	return _motionManager->StartMotionPriority(FindPtr, false, Priority);
}

float FUnLive2DRawModel::StartMotion(UUnLive2DMotion* InMotion)
{
	if (InMotion == nullptr) return 0.f;

	const FUnLive2DMotionData* MotionData = InMotion->GetMotionData();

	int32 Priority = (int32)MotionData->MotionGroupType;

	const FString Name = MotionData->GetMotionName();

	Csm::ACubismMotion* FindPtr = Live2DMotions.FindOrAdd(*Name);

	CurrentPlayMotion = InMotion;

	if (FindPtr == nullptr)
	{
		Csm::CubismMotion* Motion = static_cast<CubismMotion*>(LoadMotion(MotionData->MotionByteData.GetData(), MotionData->MotionByteData.Num(), NULL, OnMotionEnd));

		FindPtr = Motion;

		if (MotionData->FadeInTime >= 0.0f)
		{
			Motion->SetFadeInTime(MotionData->FadeInTime);
		}

		if (MotionData->FadeOutTime >= 0.0f)
		{
			Motion->SetFadeOutTime(MotionData->FadeOutTime);
		}
		Motion->SetEffectIds(EyeBlinkIds, LipSyncIds);
	}

	_motionManager->StartMotionPriority(FindPtr, false, Priority);
	return 0.f;
}

float FUnLive2DRawModel::StartExpressions(UUnLive2DExpression* InExpressions)
{
	if (InExpressions == nullptr) return 0.f;

	const FUnLiveByteData* Data = InExpressions->GetExpressionData();

	Csm::ACubismMotion* FindPtr = Live2DExpressions.FindOrAdd(InExpressions->GetUniqueID());

	if (FindPtr == nullptr)
	{
		Csm::CubismExpressionMotion* Expression = static_cast<CubismExpressionMotion*>(LoadExpression(Data->ByteData.GetData(), Data->ByteData.Num(), TCHAR_TO_UTF8(*InExpressions->GetFName().ToString())));

		FindPtr = Expression;
	}

	_expressionManager->StartMotionPriority(FindPtr, false, PriorityForce);

	return 0.f;
}

Csm::CubismMotionQueueEntryHandle FUnLive2DRawModel::StartRandomMotion(const Csm::csmChar* Group, Csm::csmInt32 Priority)
{
	if (0 == Live2DModelSetting->GetMotionCount(Group))
	{
		return InvalidMotionQueueEntryHandleValue;
	}

	csmInt32 No = FMath::Rand() % Live2DModelSetting->GetMotionCount(Group);

	return StartMotion(Group, No, Priority);
}

void FUnLive2DRawModel::SetRandomExpression()
{
	if (Live2DExpressions.Num() == 0) return;

	int32 RandIndex = FMath::RandRange(0, Live2DExpressions.Num() - 1);

	int32 i = 0;
	for (TMap<uint32, Csm::ACubismMotion*>::TConstIterator Iterator(Live2DExpressions); Iterator; ++Iterator)
	{
		if (i == RandIndex)
		{
			SetExpression(Iterator.Key());
			return;
		}
		i++;
	}
}

void FUnLive2DRawModel::SetExpression(const uint32& ExpressionID)
{
	Csm::ACubismMotion** MotionPtr = Live2DExpressions.Find(ExpressionID);

	if (MotionPtr == nullptr || *MotionPtr == nullptr) return;

	_expressionManager->StartMotionPriority(*MotionPtr, false, PriorityForce);
}