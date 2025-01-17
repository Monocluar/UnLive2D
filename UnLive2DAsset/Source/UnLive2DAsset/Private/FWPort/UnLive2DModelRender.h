#pragma once

#include "Live2DCubismCore.hpp"
#include "Rendering/CubismRenderer.hpp"
#include "Type/csmRectF.hpp"
#include "Type/csmVector.hpp"

using namespace Csm;
using Rendering::CubismRenderer;


#if ENGINE_MAJOR_VERSION < 5
typedef FMatrix FUnLiveMatrix;
typedef FVector4 FUnLiveVector4;
#else
typedef FMatrix44f FUnLiveMatrix;
typedef FVector4f FUnLiveVector4;
#endif

/**
 * 剪裁的上下文
 */
class CubismClippingContext
{
    friend class CubismClippingManager_UE;

public:
    CubismClippingContext(CubismClippingManager_UE* Manager, const csmInt32* ClippingDrawableIndices, csmInt32 ClipCount);

    virtual ~CubismClippingContext();

public:
    /**
     * 添加要被剪切到此绘图对象
     *
     * @param DrawableIndex   ->  要添加到剪切对象的绘图对象的索引
     */
    void AddClippedDrawable(csmInt32 DrawableIndex);

    /**
     * 获取管理此剪切管理器实例。
     *
     * @return  剪切管理器实例
     */
    CubismClippingManager_UE* GetClippingManager();

    csmBool _isUsing;                                ///< 如果在当前绘制状态下需要准备
    const csmInt32* _clippingIdList;                 ///< 剪裁ID列表
    csmInt32 _clippingIdCount;                       ///< 裁剪ID的数量
    csmInt32 _layoutChannelNo;                       ///< 在RGBA的哪个通道配置这个剪切(0:R , 1:G , 2:B , 3:A)
    csmRectF* _layoutBounds;                         ///< 在层级通道的哪个区域安装掩码（View坐标-1.1，UV改为0.1）
    csmRectF* _allClippedDrawRect;                   ///< 通过剪切，所有要消减的绘图对象的矩形框（每次更新）
    CubismMatrix44 _matrixForMask;                   ///< 保存遮罩位置计算结果的矩阵
    CubismMatrix44 _matrixForDraw;                   ///< 保存绘图对象位置计算结果的矩阵
    csmVector<csmInt32>* _clippedDrawableIndexList;  ///< 列表将被裁剪到此遮罩中的绘图对象

    CubismClippingManager_UE* _owner;        ///< 管理此遮罩的管理实例
};

// 裁剪处理
class CubismClippingManager_UE
{

    friend struct FCubismSepRender;

    struct FUnLive2DMaskVertex
    {
        FVector2f Position;
		FVector2f UV;
        FVector4f ViewPos;
        FLinearColor ClipColor;
    };
public:
	class FUnLive2DMaskVertexDeclaration : public FRenderResource
	{
	public:

		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual ~FUnLive2DMaskVertexDeclaration() {}

		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
		virtual void ReleaseRHI() override;
	};

public:

	static FUnLiveMatrix ConvertCubismMatrix(Csm::CubismMatrix44& InCubismMartix);

    CubismClippingManager_UE();

    virtual ~CubismClippingManager_UE();

protected:
    // 查看顶点缓存是否有变化
    bool IsVertexPositionsDidChange(CubismModel* UnLive2DModel) const;

public:

    // 获取颜色通道(0:R , 1:G , 2:B, 3:A)
    CubismRenderer::CubismTextureColor* GetChannelFlagAsColor(csmInt32 ChannelNo);

	CubismRenderer::CubismTextureColor* GetChannelFlagAsColorByDrawableIndex(const uint16& InDrawableIndex);

    bool GetFillMaskMartixForMask(const uint16& InDrawableIndex, FUnLiveMatrix& OutMartixForMask, FVector4f& OutMaskPos);

    void RenderMask_Full(FRHICommandListImmediate& RHICmdList, CubismModel* UnLive2DModel, ERHIFeatureLevel::Type FeatureLevel, FTextureRHIRef OutMaskBuffer);

    /**
     * 计算覆盖整个绘图对象的矩形（模型坐标系）
     *
     * @param Model            ->  模型实例
     * @param ClippingContext  ->  裁剪实例
     */
    void CalcClippedDrawTotalBounds(CubismModel* Model, CubismClippingContext* ClippingContext);

    /**
     * 管理器初始化处理，
     * 使用限幅掩码注册绘图对象
     *
     * @param Model           ->  模型实例
     * @param DrawableCount   ->  绘图对象的数量
     * @param DrawableMasks   ->  绘图对象的索引列表
     * @param DrawableMaskCounts   ->  要遮罩绘图对象的绘图对象数量
     */
    void Initialize(CubismModel* Model, csmInt32 DrawableCount, const csmInt32** DrawableMasks, const csmInt32* DrawableMaskCounts);

    /**
     * 创建消减上下文。绘制模型时执行。
     *
     * @param Model       ->  模型实例
     * @param Renderer    ->  渲染实例
     */
    void SetupClippingContext(CubismModel* Model, class FUnLive2DRenderState* Renderer = nullptr);

    /**
     * 如果正在制作，则返回相应的剪裁实例，
     * 如果没有制作的话会返还NULL
     *
     * @param DrawableMasks    ->  绘图对象列表
     * @param DrawableMaskCounts ->  要遮罩绘图对象的绘图对象数量
     * @return 如果存在相应的限幅掩码，则返回实例，否则返回NULL。
     */
    CubismClippingContext* FindSameClip(const csmInt32* DrawableMasks, csmInt32 DrawableMaskCounts) const;

    /**
     * 用于设置上下文的布局层级，
     * 如果掩码组的数量在4以下，则在RGBA各信道上各配置一个掩码，如果5以上6以下，则将RGBA配置为2、2、1。
     *
     * @param UsingClipCount  ->  要放置的上下文布局层级
     */
    bool SetupLayoutBounds(csmInt32 UsingClipCount) const;

    /**
     * 获取彩色缓冲区的地址
     *
     * @return 颜色缓冲区地址
     */
    //CubismOffscreenFrame_UE* GetColorBuffer() const;

    /**
     * 获取用于绘制屏幕的剪裁实例列表
     *
     * @return  用于绘制屏幕的剪裁实例列表
     */
    csmVector<CubismClippingContext*>* GetClippingContextListForDraw();

    /**
     * 设置裁切缓冲区的大小
     *
     *@param Size -> 裁剪掩码缓冲区大小
     *
     */
    void SetClippingMaskBufferSize(csmInt32 Size);

    /**
     * 获取裁切缓冲区的大小
     *
     *@return 裁剪掩码缓冲区大小
     *
     */
    csmInt32 GetClippingMaskBufferSize() const;

    CubismClippingContext* GetClipContextInDrawableIndex(const uint16& InDrawableIndex) const;

    csmInt32    _currentFrameNo;         ///< 用于蒙版纹理的框架编号

    csmVector<CubismRenderer::CubismTextureColor*>  _channelColors;

    /**
     * 这下面的两个数组是指向同样的数据
     * 只不过Mask是单独渲染的
     * Draw是跟着Order一起渲染的
     */
    csmVector<CubismClippingContext*>               _clippingContextListForMask;   ///< 掩码剪裁上下文列表
    csmVector<CubismClippingContext*>               _clippingContextListForDraw;   ///< 绘图剪裁上下文列表
    csmInt32                                        _clippingMaskBufferSize; ///< 限幅掩码的缓冲大小（初始值：256）

    CubismMatrix44  _tmpMatrix;              ///< 计算矩阵
    CubismMatrix44  _tmpMatrixForMask;       ///< 遮罩计算矩阵
    CubismMatrix44  _tmpMatrixForDraw;       ///< 绘画计算矩阵
    csmRectF        _tmpBoundsOnModel;       ///< 用于计算模型放置的矩形

private:
    struct FUnLive2DRenderBufferInfo
    {
        //FBufferRHIRef IndexBufferRHI; // 缓存遮罩顶点索引
        FBufferRHIRef VertexBufferRHI; // 缓存遮罩顶点数据
        csmUint32 ClipIndex;
        uint16 NumVertext;
		uint16 NumPrimitives;
    };

    TArray<FBufferRHIRef> CacheIndexBufferRHI; // 缓存遮罩顶点索引

    TArray<FUnLive2DRenderBufferInfo> CacheRenderBufferRHI;

};
