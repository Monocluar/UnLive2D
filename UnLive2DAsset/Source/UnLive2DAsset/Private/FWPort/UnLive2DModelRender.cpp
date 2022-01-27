#include "UnLive2DModelRender.h"
#include "Model/CubismModel.hpp"
#include "Draw/UnLive2DSepRenderer.h"


namespace {
    const csmInt32 ColorChannelCount = 4;   ///< 实验时1频道的情况是1，RGB的情况是3，Alpha也包含的情况是4
}

CubismClippingManager_UE::CubismClippingManager_UE()
    : _currentFrameNo(0)
    , _clippingMaskBufferSize(256)
{
    CubismRenderer::CubismTextureColor* tmp = NULL;
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 1.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 1.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 1.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 1.0f;
    _channelColors.PushBack(tmp);

}

CubismClippingManager_UE::~CubismClippingManager_UE()
{
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        if (_clippingContextListForMask[i]) CSM_DELETE_SELF(CubismClippingContext, _clippingContextListForMask[i]);
        _clippingContextListForMask[i] = NULL;
    }

    for (csmUint32 i = 0; i < _clippingContextListForDraw.GetSize(); i++)
    {
        _clippingContextListForDraw[i] = NULL;
    }

    for (csmUint32 i = 0; i < _channelColors.GetSize(); i++)
    {
        if (_channelColors[i]) CSM_DELETE(_channelColors[i]);
        _channelColors[i] = NULL;
    }
}

void CubismClippingManager_UE::Initialize(CubismModel* model, csmInt32 drawableCount, const csmInt32** drawableMasks, const csmInt32* drawableMaskCounts)
{
    //使用限幅遮罩注册所有绘图对象
    for (csmInt32 i = 0; i < drawableCount; i++)
    {
        if (drawableMaskCounts[i] <= 0)
        {
            //未使用限幅掩码的艺术网格（多数情况下不使用）
            _clippingContextListForDraw.PushBack(NULL);
            continue;
        }

        // 检查是否与已存在的ClipContext相同
        CubismClippingContext* cc = FindSameClip(drawableMasks[i], drawableMaskCounts[i]);
        if (cc == NULL)
        {
            // 如果不存在同一掩码，则生成
            cc = CSM_NEW CubismClippingContext(this, drawableMasks[i], drawableMaskCounts[i]);
            _clippingContextListForMask.PushBack(cc);
        }

        cc->AddClippedDrawable(i);

        _clippingContextListForDraw.PushBack(cc);
    }
}

CubismClippingContext* CubismClippingManager_UE::FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const
{
    // 确认是否与已创建的ClipingContext一致
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        CubismClippingContext* cc = _clippingContextListForMask[i];
        const csmInt32 count = cc->_clippingIdCount;
        if (count != drawableMaskCounts) continue; //个数不同的情况下是不同的东西
        csmInt32 samecount = 0;

        // 确认是否拥有相同ID。因为排列的数量相同，所以如果一致的个数相同的话，就拿相同的东西。
        for (csmInt32 j = 0; j < count; j++)
        {
            const csmInt32 clipId = cc->_clippingIdList[j];
            for (csmInt32 k = 0; k < count; k++)
            {
                if (drawableMasks[k] == clipId)
                {
                    samecount++;
                    break;
                }
            }
        }
        if (samecount == count)
        {
            return cc;
        }
    }
    return NULL; //見つからなかった
}

void CubismClippingManager_UE::SetupClippingContext(CubismModel* InModel, struct FUnLive2DRenderState* Renderer)
{
	_currentFrameNo++;

	// 全てのクリッピングを用意する
	// 同じクリップ（複数の場合はまとめて１つのクリップ）を使う場合は１度だけ設定する
	csmInt32 usingClipCount = 0;
	for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
	{
		// １つのクリッピングマスクに関して
		CubismClippingContext* cc = _clippingContextListForMask[clipIndex];

		// このクリップを利用する描画オブジェクト群全体を囲む矩形を計算
		CalcClippedDrawTotalBounds(InModel, cc);

		if (cc->_isUsing)
		{
			usingClipCount++; //使用中としてカウント
		}
	}

	// マスク作成処理
	if (usingClipCount > 0)
	{
		//if (!renderer->IsUsingHighPrecisionMask())
		//{
		//    // ビューポートは退避済み
		//    // 生成したFrameBufferと同じサイズでビューポートを設定
		//    CubismRenderer_D3D11::GetRenderStateManager()->SetViewport(renderContext,
		//        0,
		//        0,
		//        static_cast<FLOAT>(_clippingMaskBufferSize),
		//        static_cast<FLOAT>(_clippingMaskBufferSize),
		//        0.0f, 1.0f);

		//    useTarget.BeginDraw(renderContext);
		//    // 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダで Cd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
		//    useTarget.Clear(renderContext, 1.0f, 1.0f, 1.0f, 1.0f);
		//}

		// 各マスクのレイアウトを決定していく
		const bool tb_SetupGood = SetupLayoutBounds(Renderer->GetUseHighPreciseMask() ? 0 : usingClipCount);
		if (!tb_SetupGood)
		{
			Renderer->NoLowPreciseMask(true);
		}

		// 実際にマスクを生成する
		// 全てのマスクをどの様にレイアウトして描くかを決定し、ClipContext , ClippedDrawContext に記憶する
		for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
		{
			// --- 実際に１つのマスクを描く ---
			CubismClippingContext* clipContext = _clippingContextListForMask[clipIndex];
			csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
			csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds; //この中にマスクを収める

			// モデル座標上の矩形を、適宜マージンを付けて使う
			const csmFloat32 MARGIN = 0.05f;
			_tmpBoundsOnModel.SetRect(allClippedDrawRect);
			_tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, allClippedDrawRect->Height * MARGIN);
			//########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい

			// シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
			// movePeriod' = movePeriod * scaleX + offX [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
			const csmFloat32 scaleX = layoutBoundsOnTex01->Width / _tmpBoundsOnModel.Width;
			const csmFloat32 scaleY = layoutBoundsOnTex01->Height / _tmpBoundsOnModel.Height;

			// マスク生成時に使う行列を求める
			{
				// シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
				_tmpMatrix.LoadIdentity();
				{
					// Layout0..1 を -1..1に変換
					_tmpMatrix.TranslateRelative(-1.0f, -1.0f);
					_tmpMatrix.ScaleRelative(2.0f, 2.0f);
				}
				{
					// view to Layout0..1
					_tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y); //new = [translate]
					_tmpMatrix.ScaleRelative(scaleX, scaleY); //new = [translate][scale]
					_tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
					//new = [translate][scale][translate]
				}
				// tmpMatrixForMask が計算結果
				_tmpMatrixForMask.SetMatrix(_tmpMatrix.GetArray());
			}

			//--------- draw時の mask 参照用行列を計算
			{
				// シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
				_tmpMatrix.LoadIdentity();
				{
					_tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y); //new = [translate]
					// 上下反転
					_tmpMatrix.ScaleRelative(scaleX, scaleY * -1.0f); //new = [translate][scale]
					_tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
					//new = [translate][scale][translate]
				}

				_tmpMatrixForDraw.SetMatrix(_tmpMatrix.GetArray());
			}

			clipContext->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());

			clipContext->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());

	    }
    }
}

void CubismClippingManager_UE::CalcClippedDrawTotalBounds(CubismModel* model, CubismClippingContext* clippingContext)
{
    if (model == nullptr) return;

    // 被クリッピングマスク（マスクされる描画オブジェクト）の全体の矩形
    csmFloat32 clippedDrawTotalMinX = FLT_MAX, clippedDrawTotalMinY = FLT_MAX;
    csmFloat32 clippedDrawTotalMaxX = FLT_MIN, clippedDrawTotalMaxY = FLT_MIN;

    // このマスクが実際に必要か判定する
    // このクリッピングを利用する「描画オブジェクト」がひとつでも使用可能であればマスクを生成する必要がある

    const csmInt32 clippedDrawCount = clippingContext->_clippedDrawableIndexList->GetSize();
    for (csmInt32 clippedDrawableIndex = 0; clippedDrawableIndex < clippedDrawCount; clippedDrawableIndex++)
    {
        // マスクを使用する描画オブジェクトの描画される矩形を求める
        const csmInt32 drawableIndex = (*clippingContext->_clippedDrawableIndexList)[clippedDrawableIndex];

        const csmInt32 drawableVertexCount = model->GetDrawableVertexCount(drawableIndex);
        const csmFloat32* drawableVertexes = const_cast<csmFloat32*>(model->GetDrawableVertices(drawableIndex));

        csmFloat32 minX = FLT_MAX, minY = FLT_MAX;
        csmFloat32 maxX = FLT_MIN, maxY = FLT_MIN;

        csmInt32 loop = drawableVertexCount * Constant::VertexStep;
        for (csmInt32 pi = Constant::VertexOffset; pi < loop; pi += Constant::VertexStep)
        {
            csmFloat32 x = drawableVertexes[pi];
            csmFloat32 y = drawableVertexes[pi + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        //
        if (minX == FLT_MAX) continue; //有効な点がひとつも取れなかったのでスキップする

        // 全体の矩形に反映
        if (minX < clippedDrawTotalMinX) clippedDrawTotalMinX = minX;
        if (minY < clippedDrawTotalMinY) clippedDrawTotalMinY = minY;
        if (maxX > clippedDrawTotalMaxX) clippedDrawTotalMaxX = maxX;
        if (maxY > clippedDrawTotalMaxY) clippedDrawTotalMaxY = maxY;
    }
    if (clippedDrawTotalMinX == FLT_MAX)
    {
        clippingContext->_allClippedDrawRect->X = 0.0f;
        clippingContext->_allClippedDrawRect->Y = 0.0f;
        clippingContext->_allClippedDrawRect->Width = 0.0f;
        clippingContext->_allClippedDrawRect->Height = 0.0f;
        clippingContext->_isUsing = false;
    }
    else
    {
        clippingContext->_isUsing = true;
        csmFloat32 w = clippedDrawTotalMaxX - clippedDrawTotalMinX;
        csmFloat32 h = clippedDrawTotalMaxY - clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->X = clippedDrawTotalMinX;
        clippingContext->_allClippedDrawRect->Y = clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->Width = w;
        clippingContext->_allClippedDrawRect->Height = h;
    }
}

bool CubismClippingManager_UE::SetupLayoutBounds(csmInt32 usingClipCount) const
{
    if (usingClipCount <= 0)
    {// この場合は一つのマスクターゲットを毎回クリアして使用する
        for (csmUint32 index = 0; index < _clippingContextListForMask.GetSize(); index++)
        {
            CubismClippingContext* cc = _clippingContextListForMask[index];
            cc->_layoutChannelNo = 0; // どうせ毎回消すので固定で良い
            cc->_layoutBounds->X = 0.0f;
            cc->_layoutBounds->Y = 0.0f;
            cc->_layoutBounds->Width = 1.0f;
            cc->_layoutBounds->Height = 1.0f;
        }
        return true;
    }

    // ひとつのRenderTextureを極力いっぱいに使ってマスクをレイアウトする
    // マスクグループの数が4以下ならRGBA各チャンネルに１つずつマスクを配置し、5以上6以下ならRGBAを2,2,1,1と配置する

    // RGBAを順番に使っていく。
    const csmInt32 div = usingClipCount / ColorChannelCount; //１チャンネルに配置する基本のマスク個数
    const csmInt32 mod = usingClipCount % ColorChannelCount; //余り、この番号のチャンネルまでに１つずつ配分する

    // RGBAそれぞれのチャンネルを用意していく(0:R , 1:G , 2:B, 3:A, )
    csmInt32 curClipIndex = 0; //順番に設定していくk

    for (csmInt32 channelNo = 0; channelNo < ColorChannelCount; channelNo++)
    {
        // このチャンネルにレイアウトする数
        const csmInt32 layoutCount = div + (channelNo < mod ? 1 : 0);

        // 分割方法を決定する
        if (layoutCount == 0)
        {
            // 何もしない
        }
        else if (layoutCount == 1)
        {
            //全てをそのまま使う
            CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
            cc->_layoutChannelNo = channelNo;
            cc->_layoutBounds->X = 0.0f;
            cc->_layoutBounds->Y = 0.0f;
            cc->_layoutBounds->Width = 1.0f;
            cc->_layoutBounds->Height = 1.0f;
        }
        else if (layoutCount == 2)
        {
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 2;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos * 0.5f;
                cc->_layoutBounds->Y = 0.0f;
                cc->_layoutBounds->Width = 0.5f;
                cc->_layoutBounds->Height = 1.0f;
                //UVを2つに分解して使う
            }
        }
        else if (layoutCount <= 4)
        {
            //4分割して使う
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 2;
                const csmInt32 ypos = i / 2;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos * 0.5f;
                cc->_layoutBounds->Y = ypos * 0.5f;
                cc->_layoutBounds->Width = 0.5f;
                cc->_layoutBounds->Height = 0.5f;
            }
        }
        else if (layoutCount <= 9)
        {
            //9分割して使う
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 3;
                const csmInt32 ypos = i / 3;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos / 3.0f;
                cc->_layoutBounds->Y = ypos / 3.0f;
                cc->_layoutBounds->Width = 1.0f / 3.0f;
                cc->_layoutBounds->Height = 1.0f / 3.0f;
            }
        }
        else
        {
            CubismLogError("not supported mask count : %d", layoutCount);

            // 開発モードの場合は停止させる
            CSM_ASSERT(0);

            // 引き続き実行する場合、 SetupShaderProgramでオーバーアクセスが発生するので仕方なく適当に入れておく
            // もちろん描画結果はろくなことにならない
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = 0;
                cc->_layoutBounds->X = 0.0f;
                cc->_layoutBounds->Y = 0.0f;
                cc->_layoutBounds->Width = 1.0f;
                cc->_layoutBounds->Height = 1.0f;
            }

            return false;
        }
    }

    return true;
}

CubismRenderer::CubismTextureColor* CubismClippingManager_UE::GetChannelFlagAsColor(csmInt32 channelNo)
{
    return _channelColors[channelNo];
}

//CubismOffscreenFrame_D3D11* CubismClippingManager_UE::GetColorBuffer() const
//{
//    return _colorBuffer;
//}

csmVector<CubismClippingContext*>* CubismClippingManager_UE::GetClippingContextListForDraw()
{
    return &_clippingContextListForDraw;
}

void CubismClippingManager_UE::SetClippingMaskBufferSize(csmInt32 size)
{
    _clippingMaskBufferSize = size;
}

csmInt32 CubismClippingManager_UE::GetClippingMaskBufferSize() const
{
    return _clippingMaskBufferSize;
}

/*********************************************************************************************************************
*                                      CubismClippingContext
********************************************************************************************************************/
CubismClippingContext::CubismClippingContext(CubismClippingManager_UE* manager, const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
{
    _isUsing = false;

    _owner = manager;

    // クリップしている（＝マスク用の）Drawableのインデックスリスト
    _clippingIdList = clippingDrawableIndices;

    // マスクの数
    _clippingIdCount = clipCount;

    _layoutChannelNo = 0;

    _allClippedDrawRect = CSM_NEW csmRectF();
    _layoutBounds = CSM_NEW csmRectF();

    _clippedDrawableIndexList = CSM_NEW csmVector<csmInt32>();
}

CubismClippingContext::~CubismClippingContext()
{
    if (_layoutBounds != NULL)
    {
        CSM_DELETE(_layoutBounds);
        _layoutBounds = NULL;
    }

    if (_allClippedDrawRect != NULL)
    {
        CSM_DELETE(_allClippedDrawRect);
        _allClippedDrawRect = NULL;
    }

    if (_clippedDrawableIndexList != NULL)
    {
        CSM_DELETE(_clippedDrawableIndexList);
        _clippedDrawableIndexList = NULL;
    }
}

void CubismClippingContext::AddClippedDrawable(csmInt32 drawableIndex)
{
    _clippedDrawableIndexList->PushBack(drawableIndex);
}

CubismClippingManager_UE* CubismClippingContext::GetClippingManager()
{
    return _owner;
}

