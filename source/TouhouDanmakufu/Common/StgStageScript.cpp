#include "source/GcLib/pch.h"

#include "StgStageScript.hpp"
#include "StgSystem.hpp"
#include "StgPlayer.hpp"
#include "StgShot.hpp"
#include "StgItem.hpp"

/**********************************************************
//StgStageScriptManager
**********************************************************/
StgStageScriptManager::StgStageScriptManager(StgStageController* stageController) {
	stageController_ = stageController;
	objManager_ = stageController_->GetMainObjectManagerRef();
	idPlayerScript_ = ID_INVALID;
	idItemScript_ = ID_INVALID;
	idShotScript_ = ID_INVALID;

	ptrPlayerScript_ = nullptr;
	ptrItemScript_ = nullptr;
	ptrShotScript_ = nullptr;
}
StgStageScriptManager::~StgStageScriptManager() {

}

void StgStageScriptManager::SetError(std::wstring error) {
	StgControlScriptManager::SetError(error);
	stageController_->GetSystemInformation()->SetError(error);
}
bool StgStageScriptManager::IsError() {
	bool res = error_ != L"" || stageController_->GetSystemInformation()->IsError();
	return res;
}

shared_ptr<ManagedScript> StgStageScriptManager::Create(int type) {
	shared_ptr<ManagedScript> res = nullptr;
	switch (type) {
	case StgStageScript::TYPE_STAGE:
		res = std::make_shared<StgStageScript>(stageController_);
		break;
	case StgStageScript::TYPE_SYSTEM:
		res = std::make_shared<StgStageSystemScript>(stageController_);
		break;
	case StgStageScript::TYPE_ITEM:
		res = std::make_shared<StgStageItemScript>(stageController_);
		break;
	case StgStageScript::TYPE_SHOT:
		res = std::make_shared<StgStageShotScript>(stageController_);
		break;
	case StgStageScript::TYPE_PLAYER:
		res = std::make_shared<StgStagePlayerScript>(stageController_);
		break;
	}
	if (res)
		res->SetScriptManager(stageController_->GetScriptManager());
	return res;
}

void StgStageScriptManager::SetPlayerScript(shared_ptr<ManagedScript> id) {
	ptrPlayerScript_ = id;
	idPlayerScript_ = id->GetScriptID();
}
void StgStageScriptManager::SetItemScript(shared_ptr<ManagedScript> id) {
	ptrItemScript_ = id;
	idItemScript_ = id->GetScriptID();
}
void StgStageScriptManager::SetShotScript(shared_ptr<ManagedScript> id) {
	ptrShotScript_ = id;
	idShotScript_ = id->GetScriptID();
}


/**********************************************************
//StgStageScriptObjectManager
**********************************************************/
StgStageScriptObjectManager::StgStageScriptObjectManager(StgStageController* stageController) {
	stageController_ = stageController;
	SetMaxObject(DEFAULT_CONTAINER_CAPACITY);

	idObjPlayer_ = DxScript::ID_INVALID;
	ptrObjPlayer_ = nullptr;
}
StgStageScriptObjectManager::~StgStageScriptObjectManager() {
	/*
	if (idObjPlayer_ != DxScript::ID_INVALID) {
		shared_ptr<StgPlayerObject> obj = std::dynamic_pointer_cast<StgPlayerObject>(GetObject(idObjPlayer_));
		if (obj != nullptr)
			obj->Clear();
	}
	*/
	if (ptrObjPlayer_) ptrObjPlayer_->Clear();
}
/*
void StgStageScriptObjectManager::PrepareRenderObject() {
	for (auto itr = listActiveObject_.begin(); itr != listActiveObject_.end(); ++itr) {
		DxScriptObjectBase* obj = itr->get();
		if (obj == nullptr || obj->IsDeleted()) continue;
		//Shots and items are already sorted in StgShotManager and StgItemManager
		//if (dynamic_cast<StgShotObject*>(obj) != nullptr || dynamic_cast<StgItemObject*>(obj) != nullptr) continue;
		if (!obj->HasNormalRendering()) continue;
		if (!obj->IsVisible()) continue;
		AddRenderObject(*itr);
	}
}
*/
void StgStageScriptObjectManager::RenderObject() {
	/*
		if(invalidPriMin_ < 0 && invalidPriMax_ < 0)
		{
			RenderObject(0, objRender_.size());
		}
		else
		{
			RenderObject(0, invalidPriMin_);
			RenderObject(invalidPriMax_ + 1, objRender_.size());
		}
	*/
}

void StgStageScriptObjectManager::RenderObject(int priMin, int priMax) {
	/*
		std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
		for(itr = listActiveObject_.begin() ; itr != listActiveObject_.end() ; itr++)
		{
			gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = (*itr);
			if(obj == nullptr || obj->IsDeleted()) continue;
			if(!obj->IsVisible()) continue;
			AddRenderObject(obj);
		}

		DirectGraphics* graphics = DirectGraphics::GetBase();
		gstd::ref_count_ptr<DxCamera> camera3D = graphics->GetCamera();
		gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();

		ref_count_ptr<StgStageInformation> stageInfo = stageController_->GetStageInformation();
		RECT rcStgFrame = stageInfo->GetStgFrameRect();
		int stgWidth = rcStgFrame.right - rcStgFrame.left;
		int stgHeight = rcStgFrame.bottom - rcStgFrame.top;
		POINT stgCenter = {rcStgFrame.left + stgWidth / 2, rcStgFrame.top + stgHeight / 2};
		int priMinStgFrame = stageInfo->GetStgFrameMinPriority();
		int priMaxStgFrame = stageInfo->GetStgFrameMaxPriority();
		int priShot = stageInfo->GetShotObjectPriority();
		int priItem = stageInfo->GetItemObjectPriority();
		int priCamera = stageInfo->GetCameraFocusPermitPriority();

		double focusRatio = camera2D->GetRatio();
		D3DXVECTOR2 orgFocusPos = camera2D->GetFocusPosition();
		D3DXVECTOR2 focusPos = orgFocusPos;
		focusPos.x -= stgWidth / 2;
		focusPos.y -= stgHeight / 2;

		//フォグ設定
		graphics->SetVertexFog(bFogEnable_, fogColor_, fogStart_, fogEnd_);

		//描画開始前リセット
		camera2D->SetEnable(false);
		camera2D->Reset();
		graphics->ResetViewPort();

		bool bClearZBufferFor2DCoordinate = false;
		bool bRunMinStgFrame = false;
		bool bRunMaxStgFrame = false;
		for(int iPri = priMin ; iPri <= priMax ; iPri++)
		{
			if(iPri >= objRender_.size()) break;

			if(iPri >= priMinStgFrame && !bRunMinStgFrame)
			{
				//STGフレーム開始
				graphics->ClearRenderTarget(rcStgFrame);
				camera2D->SetEnable(true);
				camera2D->SetRatio(focusRatio);
				camera2D->SetClip(rcStgFrame);
				camera2D->SetFocus(stgCenter.x + focusPos.x, stgCenter.y + focusPos.y);
				camera3D->SetProjectionMatrix(rcStgFrame.right - rcStgFrame.left, rcStgFrame.bottom - rcStgFrame.top, 10, 2000);
				camera3D->UpdateDeviceProjectionMatrix();

				graphics->SetViewPort(rcStgFrame.left, rcStgFrame.top, stgWidth, stgHeight);

				bRunMinStgFrame = true;
				bClearZBufferFor2DCoordinate = false;
			}
			if(iPri == priShot)
			{
				//弾描画
				stageController_->GetShotManager()->Render();
			}
			if(iPri == priItem)
			{
				//アイテム描画
				stageController_->GetItemManager()->Render();
			}

			std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
			for(itr = objRender_[iPri].begin() ; itr != objRender_[iPri].end() ; itr++)
			{
				if(!bClearZBufferFor2DCoordinate)
				{
					DxScriptMeshObject* objMesh = dynamic_cast<DxScriptMeshObject*>((*itr).GetPointer());
					if(objMesh != nullptr)
					{
						shared_ptr<DxMesh>& mesh = objMesh->GetMesh();
						if(mesh != nullptr && mesh->IsCoordinate2D())
						{
							graphics->GetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0,0);
							bClearZBufferFor2DCoordinate = true;
						}
					}
				}
				(*itr)->Render();
			}
			objRender_[iPri].clear();

			if(iPri == priCamera)
			{
				camera2D->SetFocus(stgCenter.x, stgCenter.y);
				camera2D->SetRatio(1);
			}
			if(iPri >= priMaxStgFrame && !bRunMaxStgFrame)
			{
				//STGフレーム終了
				camera2D->SetEnable(false);
				camera2D->Reset();
				graphics->ResetViewPort();

				bRunMaxStgFrame = true;
				bClearZBufferFor2DCoordinate = false;
			}
		}
		camera2D->SetFocus(orgFocusPos);
		camera2D->SetRatio(focusRatio);
	*/
}
int StgStageScriptObjectManager::CreatePlayerObject() {
	//自機オブジェクト生成
	ptrObjPlayer_ = shared_ptr<StgPlayerObject>(new StgPlayerObject(stageController_));
	idObjPlayer_ = AddObject(ptrObjPlayer_);
	return idObjPlayer_;
}


/**********************************************************
//StgStageScript
**********************************************************/
function const stgFunction[] =
{
	//STG共通関数：共通データ
	{ "SaveCommonDataAreaToReplayFile", StgStageScript::Func_SaveCommonDataAreaToReplayFile, 1 },
	{ "LoadCommonDataAreaFromReplayFile", StgStageScript::Func_LoadCommonDataAreaFromReplayFile, 1 },

	//STG共通関数：システム関連
	{ "GetMainStgScriptPath", StgStageScript::Func_GetMainStgScriptPath, 0 },
	{ "GetMainStgScriptDirectory", StgStageScript::Func_GetMainStgScriptDirectory, 0 },
	{ "SetStgFrame", StgStageScript::Func_SetStgFrame, 6 },
	{ "SetItemRenderPriorityI", StgStageScript::Func_SetItemRenderPriorityI, 1 },
	{ "SetShotRenderPriorityI", StgStageScript::Func_SetShotRenderPriorityI, 1 },
	{ "GetStgFrameRenderPriorityMinI", StgStageScript::Func_GetStgFrameRenderPriorityMinI, 0 },
	{ "GetStgFrameRenderPriorityMaxI", StgStageScript::Func_GetStgFrameRenderPriorityMaxI, 0 },
	{ "GetItemRenderPriorityI", StgStageScript::Func_GetItemRenderPriorityI, 0 },
	{ "GetShotRenderPriorityI", StgStageScript::Func_GetShotRenderPriorityI, 0 },
	{ "GetPlayerRenderPriorityI", StgStageScript::Func_GetPlayerRenderPriorityI, 0 },
	{ "GetCameraFocusPermitPriorityI", StgStageScript::Func_GetCameraFocusPermitPriorityI, 0 },
	{ "CloseStgScene", StgStageScript::Func_CloseStgScene, 0 },
	{ "GetReplayFps", StgStageScript::Func_GetReplayFps, 0 },

	//STG共通関数：自機
	{ "GetPlayerObjectID", StgStageScript::Func_GetPlayerObjectID, 0 },
	{ "GetPlayerScriptID", StgStageScript::Func_GetPlayerScriptID, 0 },
	{ "SetPlayerSpeed", StgStageScript::Func_SetPlayerSpeed, 2 },
	{ "SetPlayerClip", StgStageScript::Func_SetPlayerClip, 4 },
	{ "SetPlayerLife", StgStageScript::Func_SetPlayerLife, 1 },
	{ "SetPlayerSpell", StgStageScript::Func_SetPlayerSpell, 1 },
	{ "SetPlayerPower", StgStageScript::Func_SetPlayerPower, 1 },
	{ "SetPlayerInvincibilityFrame", StgStageScript::Func_SetPlayerInvincibilityFrame, 1 },
	{ "SetPlayerDownStateFrame", StgStageScript::Func_SetPlayerDownStateFrame, 1 },
	{ "SetPlayerRebirthFrame", StgStageScript::Func_SetPlayerRebirthFrame, 1 },
	{ "SetPlayerRebirthLossFrame", StgStageScript::Func_SetPlayerRebirthLossFrame, 1 },
	{ "SetPlayerAutoItemCollectLine", StgStageScript::Func_SetPlayerAutoItemCollectLine, 1 },
	{ "GetPlayerAutoItemCollectLine", StgStageScript::Func_GetPlayerInfoAsInt<&StgPlayerObject::GetAutoItemCollectY, -1>, 0 },
	{ "SetForbidPlayerShot", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetForbidShot>, 1 },
	{ "SetForbidPlayerSpell", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetForbidSpell>, 1 },
	{ "GetPlayerX", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetX, 0>, 0 },
	{ "GetPlayerY", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetY, 0>, 0 },
	{ "GetPlayerState", StgStageScript::Func_GetPlayerInfoAsInt<&StgPlayerObject::GetState, StgPlayerObject::STATE_END>, 0 },
	{ "GetPlayerSpeed", StgStageScript::Func_GetPlayerSpeed, 0 },
	{ "GetPlayerClip", StgStageScript::Func_GetPlayerClip, 0 },
	{ "GetPlayerLife", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetLife, 0>, 0 },
	{ "GetPlayerSpell", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetSpell, 0>, 0 },
	{ "GetPlayerPower", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetPower, 0>, 0 },
	{ "GetPlayerInvincibilityFrame", StgStageScript::Func_GetPlayerInfoAsInt<&StgPlayerObject::GetInvincibilityFrame, 0>, 0 },
	{ "GetPlayerDownStateFrame", StgStageScript::Func_GetPlayerInfoAsInt<&StgPlayerObject::GetDownStateFrame, 0>, 0 },
	{ "GetPlayerRebirthFrame", StgStageScript::Func_GetPlayerInfoAsInt<&StgPlayerObject::GetRebirthFrame, 0>, 0 },
	{ "GetAngleToPlayer", StgStageScript::Func_GetAngleToPlayer, 1 },
	{ "IsPermitPlayerShot", StgStageScript::Func_IsPermitPlayerShot, 0 },
	{ "IsPermitPlayerSpell", StgStageScript::Func_IsPermitPlayerSpell, 0 },
	{ "IsPlayerLastSpellWait", StgStageScript::Func_IsPlayerLastSpellWait, 0 },
	{ "IsPlayerSpellActive", StgStageScript::Func_IsPlayerSpellActive, 0 },
	{ "SetPlayerItemScope", StgStageScript::Func_SetPlayerItemScope, 1 },
	{ "GetPlayerItemScope", StgStageScript::Func_GetPlayerInfoAsDbl<&StgPlayerObject::GetItemIntersectionRadius, -1>, 0 },
	{ "SetPlayerInvincibleGraze", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetEnableInvincibleGraze>, 1 },
	{ "SetPlayerIntersectionEraseShot", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetDeleteShotOnHit>, 1 },
	{ "SetPlayerStateEndEnable", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetEnableStateEnd>, 1 },
	{ "SetPlayerShootdownEventEnable", StgStageScript::Func_SetPlayerInfoAsBool<&StgPlayerObject::SetEnableShootdownEvent>, 1 },
	{ "SetPlayerRebirthPosition", StgStageScript::Func_SetPlayerRebirthPosition, 2 },

	//STG共通関数：敵
	{ "GetEnemyBossSceneObjectID", StgStageScript::Func_GetEnemyBossSceneObjectID, 0 },
	{ "GetEnemyBossObjectID", StgStageScript::Func_GetEnemyBossObjectID, 0 },
	{ "GetAllEnemyID", StgStageScript::Func_GetAllEnemyID, 0 },
	{ "GetIntersectionRegistedEnemyID", StgStageScript::Func_GetIntersectionRegistedEnemyID, 0 },
	{ "GetAllEnemyIntersectionPosition", StgStageScript::Func_GetAllEnemyIntersectionPosition, 0 },
	{ "GetEnemyIntersectionPosition", StgStageScript::Func_GetEnemyIntersectionPosition, 3 },
	{ "GetEnemyIntersectionPositionByIdA1", StgStageScript::Func_GetEnemyIntersectionPositionByIdA1, 1 },
	{ "GetEnemyIntersectionPositionByIdA2", StgStageScript::Func_GetEnemyIntersectionPositionByIdA2, 3 },
	{ "LoadEnemyShotData", StgStageScript::Func_LoadEnemyShotData, 1 },
	{ "ReloadEnemyShotData", StgStageScript::Func_ReloadEnemyShotData, 1 },

	//STG共通関数：弾
	{ "DeleteShotAll", StgStageScript::Func_DeleteShotAll, 2 },
	{ "DeleteShotInCircle", StgStageScript::Func_DeleteShotInCircle, 5 },
	{ "CreateShotA1", StgStageScript::Func_CreateShotA1, 6 },
	{ "CreateShotA2", StgStageScript::Func_CreateShotA2, 8 },
	{ "CreateShotOA1", StgStageScript::Func_CreateShotOA1, 5 },
	{ "CreateShotB1", StgStageScript::Func_CreateShotB1, 6 },
	{ "CreateShotB2", StgStageScript::Func_CreateShotB2, 10 },
	{ "CreateShotOB1", StgStageScript::Func_CreateShotOB1, 5 },
	{ "CreateLooseLaserA1", StgStageScript::Func_CreateLooseLaserA1, 8 },
	{ "CreateStraightLaserA1", StgStageScript::Func_CreateStraightLaserA1, 8 },
	{ "CreateCurveLaserA1", StgStageScript::Func_CreateCurveLaserA1, 8 },
	//{"StgStraightLaserA1", StgStageScript::Func_CreateStraightLaserA1, 8},
	{ "SetShotIntersectionCircle", StgStageScript::Func_SetShotIntersectionCircle, 3 },
	{ "SetShotIntersectionLine", StgStageScript::Func_SetShotIntersectionLine, 5 },
	{ "GetShotIdInCircleA1", StgStageScript::Func_GetShotIdInCircleA1, 3 },
	{ "GetShotIdInCircleA2", StgStageScript::Func_GetShotIdInCircleA2, 4 },
	{ "GetShotCount", StgStageScript::Func_GetShotCount, 1 },
	{ "SetShotAutoDeleteClip", StgStageScript::Func_SetShotAutoDeleteClip, 4 },
	{ "GetShotDataInfoA1", StgStageScript::Func_GetShotDataInfoA1, 3 },
	{ "StartShotScript", StgStageScript::Func_StartShotScript, 1 },

	//STG共通関数：アイテム
	{ "CreateItemA1", StgStageScript::Func_CreateItemA1, 4 },
	{ "CreateItemA2", StgStageScript::Func_CreateItemA2, 6 },
	{ "CreateItemU1", StgStageScript::Func_CreateItemU1, 4 },
	{ "CreateItemU2", StgStageScript::Func_CreateItemU2, 6 },
	{ "CreateItemScore", StgStageScript::Func_CreateItemScore, 3 },
	{ "CollectAllItems", StgStageScript::Func_CollectAllItems, 0 },
	{ "CollectItemsByType", StgStageScript::Func_CollectItemsByType, 1 },
	{ "CollectItemsInCircle", StgStageScript::Func_CollectItemsInCircle, 3 },
	{ "CancelCollectItems", StgStageScript::Func_CancelCollectItems, 0 },
	{ "StartItemScript", StgStageScript::Func_StartItemScript, 1 },
	{ "SetDefaultBonusItemEnable", StgStageScript::Func_SetDefaultBonusItemEnable, 1 },
	{ "LoadItemData", StgStageScript::Func_LoadItemData, 1 },
	{ "ReloadItemData", StgStageScript::Func_ReloadItemData, 1 },
	{ "SetItemIntersectionRadius", StgStageScript::Func_SetItemIntersectionRadius, 1 },

	//STG共通関数：その他
	{ "StartSlow", StgStageScript::Func_StartSlow, 2 },
	{ "StopSlow", StgStageScript::Func_StopSlow, 1 },
	{ "IsIntersected_Obj_Obj", StgStageScript::Func_IsIntersected_Obj_Obj<true>, 2 },
	{ "IsIntersected_Obj_Obj_All", StgStageScript::Func_IsIntersected_Obj_Obj<false>, 2 },

	//STG共通関数：移動オブジェクト操作
	{ "ObjMove_SetX", StgStageScript::Func_ObjMove_SetX, 2 },
	{ "ObjMove_SetY", StgStageScript::Func_ObjMove_SetY, 2 },
	{ "ObjMove_SetPosition", StgStageScript::Func_ObjMove_SetPosition, 3 },
	{ "ObjMove_SetSpeed", StgStageScript::Func_ObjMove_SetSpeed, 2 },
	{ "ObjMove_SetAngle", StgStageScript::Func_ObjMove_SetAngle, 2 },
	{ "ObjMove_SetAcceleration", StgStageScript::Func_ObjMove_SetAcceleration, 2 },
	{ "ObjMove_SetMaxSpeed", StgStageScript::Func_ObjMove_SetMaxSpeed, 2 },
	{ "ObjMove_SetAngularVelocity", StgStageScript::Func_ObjMove_SetAngularVelocity, 2 },
	{ "ObjMove_SetDestAtSpeed", StgStageScript::Func_ObjMove_SetDestAtSpeed, 4 },
	{ "ObjMove_SetDestAtFrame", StgStageScript::Func_ObjMove_SetDestAtFrame, 4 },
	{ "ObjMove_SetDestAtWeight", StgStageScript::Func_ObjMove_SetDestAtWeight, 5 },
	{ "ObjMove_AddPatternA1", StgStageScript::Func_ObjMove_AddPatternA1, 4 },
	{ "ObjMove_AddPatternA2", StgStageScript::Func_ObjMove_AddPatternA2, 7 },
	{ "ObjMove_AddPatternA3", StgStageScript::Func_ObjMove_AddPatternA3, 8 },
	{ "ObjMove_AddPatternA4", StgStageScript::Func_ObjMove_AddPatternA4, 9 },
	{ "ObjMove_AddPatternB1", StgStageScript::Func_ObjMove_AddPatternB1, 4 },
	{ "ObjMove_AddPatternB2", StgStageScript::Func_ObjMove_AddPatternB2, 8 },
	{ "ObjMove_AddPatternB3", StgStageScript::Func_ObjMove_AddPatternB3, 9 },
	{ "ObjMove_GetX", StgStageScript::Func_ObjMove_GetX, 1 },
	{ "ObjMove_GetY", StgStageScript::Func_ObjMove_GetY, 1 },
	{ "ObjMove_GetSpeed", StgStageScript::Func_ObjMove_GetSpeed, 1 },
	{ "ObjMove_GetAngle", StgStageScript::Func_ObjMove_GetAngle, 1 },
	{ "ObjMove_SetSpeedX", StgStageScript::Func_ObjMove_SetSpeedX, 2 },
	{ "ObjMove_GetSpeedX", StgStageScript::Func_ObjMove_GetSpeedX, 1 },
	{ "ObjMove_SetSpeedY", StgStageScript::Func_ObjMove_SetSpeedY, 2 },
	{ "ObjMove_GetSpeedY", StgStageScript::Func_ObjMove_GetSpeedY, 1 },
	{ "ObjMove_SetSpeedXY", StgStageScript::Func_ObjMove_SetSpeedXY, 3 },
	{ "ObjMove_SetProcessMovement", StgStageScript::Func_ObjMove_SetProcessMovement, 2 },
	{ "ObjMove_GetProcessMovement", StgStageScript::Func_ObjMove_GetProcessMovement, 1 },

	//STG共通関数：敵オブジェクト操作
	{ "ObjEnemy_Create", StgStageScript::Func_ObjEnemy_Create, 1 },
	{ "ObjEnemy_Regist", StgStageScript::Func_ObjEnemy_Regist, 1 },
	{ "ObjEnemy_GetInfo", StgStageScript::Func_ObjEnemy_GetInfo, 2 },
	{ "ObjEnemy_SetLife", StgStageScript::Func_ObjEnemy_SetLife, 2 },
	{ "ObjEnemy_AddLife", StgStageScript::Func_ObjEnemy_AddLife, 2 },
	{ "ObjEnemy_SetDamageRate", StgStageScript::Func_ObjEnemy_SetDamageRate, 3 },
	{ "ObjEnemy_AddIntersectionCircleA", StgStageScript::Func_ObjEnemy_AddIntersectionCircleA, 4 },
	{ "ObjEnemy_SetIntersectionCircleToShot", StgStageScript::Func_ObjEnemy_SetIntersectionCircleToShot, 4 },
	{ "ObjEnemy_SetIntersectionCircleToPlayer", StgStageScript::Func_ObjEnemy_SetIntersectionCircleToPlayer, 4 },
	{ "ObjEnemy_GetIntersectionCircleListToShot", StgStageScript::Func_ObjEnemy_GetIntersectionCircleToShot, 1 },
	{ "ObjEnemy_GetIntersectionCircleListToPlayer", StgStageScript::Func_ObjEnemy_GetIntersectionCircleToPlayer, 1 },
	{ "ObjEnemy_SetEnableIntersectionPositionFetching", StgStageScript::Func_ObjEnemy_SetEnableIntersectionPositionFetching, 2 },

		//STG共通関数：敵ボスシーンオブジェクト操作
	{ "ObjEnemyBossScene_Create", StgStageScript::Func_ObjEnemyBossScene_Create, 0 },
	{ "ObjEnemyBossScene_Regist", StgStageScript::Func_ObjEnemyBossScene_Regist, 1 },
	{ "ObjEnemyBossScene_Add", StgStageScript::Func_ObjEnemyBossScene_Add, 3 },
	{ "ObjEnemyBossScene_LoadInThread", StgStageScript::Func_ObjEnemyBossScene_LoadInThread, 1 },
	{ "ObjEnemyBossScene_GetInfo", StgStageScript::Func_ObjEnemyBossScene_GetInfo, 2 },
	{ "ObjEnemyBossScene_SetSpellTimer", StgStageScript::Func_ObjEnemyBossScene_SetSpellTimer, 2 },
	{ "ObjEnemyBossScene_StartSpell", StgStageScript::Func_ObjEnemyBossScene_StartSpell, 1 },
	{ "ObjEnemyBossScene_EndSpell", StgStageScript::Func_ObjEnemyBossScene_EndSpell, 1 },

	//STG共通関数：弾オブジェクト操作
	{ "ObjShot_Create", StgStageScript::Func_ObjShot_Create, 1 },
	{ "ObjShot_Regist", StgStageScript::Func_ObjShot_Regist, 1 },
	{ "ObjShot_SetOwnerType", StgStageScript::Func_ObjShot_SetOwnerType, 2 },
	{ "ObjShot_SetAutoDelete", StgStageScript::Func_ObjShot_SetAutoDelete, 2 },
	{ "ObjShot_FadeDelete", StgStageScript::Func_ObjShot_FadeDelete, 1 },
	{ "ObjShot_SetDeleteFrame", StgStageScript::Func_ObjShot_SetDeleteFrame, 2 },
	{ "ObjShot_SetDelay", StgStageScript::Func_ObjShot_SetDelay, 2 },
	{ "ObjShot_SetSpellResist", StgStageScript::Func_ObjShot_SetSpellResist, 2 },
	{ "ObjShot_SetGraphic", StgStageScript::Func_ObjShot_SetGraphic, 2 },
	{ "ObjShot_SetSourceBlendType", StgStageScript::Func_ObjShot_SetSourceBlendType, 2 },
	{ "ObjShot_SetDamage", StgStageScript::Func_ObjShot_SetDamage, 2 },
	{ "ObjShot_SetPenetration", StgStageScript::Func_ObjShot_SetPenetration, 2 },
	{ "ObjShot_SetEraseShot", StgStageScript::Func_ObjShot_SetEraseShot, 2 },
	{ "ObjShot_SetSpellFactor", StgStageScript::Func_ObjShot_SetSpellFactor, 2 },
	{ "ObjShot_ToItem", StgStageScript::Func_ObjShot_ToItem, 1 },
	{ "ObjShot_SetIntersectionCircleA1", StgStageScript::Func_ObjShot_SetIntersectionCircleA1, 2 },
	{ "ObjShot_SetIntersectionCircleA2", StgStageScript::Func_ObjShot_SetIntersectionCircleA2, 4 },
	{ "ObjShot_SetIntersectionLine", StgStageScript::Func_ObjShot_SetIntersectionLine, 6 },
	{ "ObjShot_SetIntersectionEnable", StgStageScript::Func_ObjShot_SetIntersectionEnable, 2 },
	{ "ObjShot_GetIntersectionEnable", StgStageScript::Func_ObjShot_GetIntersectionEnable, 1 },
	{ "ObjShot_SetItemChange", StgStageScript::Func_ObjShot_SetItemChange, 2 },
	{ "ObjShot_GetDelay", StgStageScript::Func_ObjShot_GetDelay, 1 },
	{ "ObjShot_GetDamage", StgStageScript::Func_ObjShot_GetDamage, 1 },
	{ "ObjShot_GetPenetration", StgStageScript::Func_ObjShot_GetPenetration, 1 },
	{ "ObjShot_IsSpellResist", StgStageScript::Func_ObjShot_IsSpellResist, 1 },
	{ "ObjShot_GetImageID", StgStageScript::Func_ObjShot_GetImageID, 1 },
	{ "ObjShot_SetIntersectionScaleX", StgStageScript::Func_ObjShot_SetIntersectionScaleX, 2 },
	{ "ObjShot_SetIntersectionScaleY", StgStageScript::Func_ObjShot_SetIntersectionScaleY, 2 },
	{ "ObjShot_SetIntersectionScaleXY", StgStageScript::Func_ObjShot_SetIntersectionScaleXY, 3 },
	{ "ObjShot_SetPositionRounding", StgStageScript::Func_ObjShot_SetPositionRounding, 2 },
	{ "ObjShot_SetDelayMotionEnable", StgStageScript::Func_ObjShot_SetDelayMotionEnable, 2 },
	{ "ObjShot_SetDelayGraphic", StgStageScript::Func_ObjShot_SetDelayGraphic, 2 },
	{ "ObjShot_SetDelayScaleParameter", StgStageScript::Func_ObjShot_SetDelayScaleParameter, 4 },
	{ "ObjShot_SetDelayAlphaParameter", StgStageScript::Func_ObjShot_SetDelayAlphaParameter, 4 },
	{ "ObjShot_SetDelayMode", StgStageScript::Func_ObjShot_SetDelayMode, 4 },
	{ "ObjShot_SetDelayColor", StgStageScript::Func_ObjShot_SetDelayColor, 2 },
	{ "ObjShot_SetDelayColoringEnable", StgStageScript::Func_ObjShot_SetDelayColoringEnable, 2 },
	{ "ObjShot_SetGrazeInvalidFrame", StgStageScript::Func_ObjShot_SetGrazeInvalidFrame, 2 },
	{ "ObjShot_SetGrazeFrame", StgStageScript::Func_ObjShot_SetGrazeFrame, 2 },
	{ "ObjShot_IsValidGraze", StgStageScript::Func_ObjShot_IsValidGraze, 1 },

	{ "ObjLaser_SetLength", StgStageScript::Func_ObjLaser_SetLength, 2 },
	{ "ObjLaser_SetRenderWidth", StgStageScript::Func_ObjLaser_SetRenderWidth, 2 },
	{ "ObjLaser_SetIntersectionWidth", StgStageScript::Func_ObjLaser_SetIntersectionWidth, 2 },
	{ "ObjLaser_SetInvalidLength", StgStageScript::Func_ObjLaser_SetInvalidLength, 3 },
	{ "ObjLaser_SetItemDistance", StgStageScript::Func_ObjLaser_SetItemDistance, 2 },
	{ "ObjLaser_GetLength", StgStageScript::Func_ObjLaser_GetLength, 1 },
	{ "ObjLaser_GetRenderWidth", StgStageScript::Func_ObjLaser_GetRenderWidth, 1 },
	{ "ObjLaser_GetIntersectionWidth", StgStageScript::Func_ObjLaser_GetIntersectionWidth, 1 },
	{ "ObjStLaser_SetAngle", StgStageScript::Func_ObjStLaser_SetAngle, 2 },
	{ "ObjStLaser_GetAngle", StgStageScript::Func_ObjStLaser_GetAngle, 1 },
	{ "ObjStLaser_SetSource", StgStageScript::Func_ObjStLaser_SetSource, 2 },
	{ "ObjStLaser_SetEnd", StgStageScript::Func_ObjStLaser_SetEnd, 2 },
	{ "ObjStLaser_SetEndGraphic", StgStageScript::Func_ObjStLaser_SetEndGraphic, 2 },
	{ "ObjStLaser_SetDelayScale", StgStageScript::Func_ObjStLaser_SetDelayScale, 3 },
	{ "ObjStLaser_SetPermitExpand", StgStageScript::Func_ObjStLaser_SetPermitExpand, 2 },
	{ "ObjStLaser_GetPermitExpand", StgStageScript::Func_ObjStLaser_GetPermitExpand, 1 },
	{ "ObjCrLaser_SetTipDecrement", StgStageScript::Func_ObjCrLaser_SetTipDecrement, 2 },
	{ "ObjCrLaser_GetNodePointer", StgStageScript::Func_ObjCrLaser_GetNodePointer, 2 },
	{ "ObjCrLaser_GetNodePointerList", StgStageScript::Func_ObjCrLaser_GetNodePointerList, 1 },
	{ "ObjCrLaser_GetNodePosition", StgStageScript::Func_ObjCrLaser_GetNodePosition, 2 },
	{ "ObjCrLaser_GetNodeAngle", StgStageScript::Func_ObjCrLaser_GetNodeAngle, 2 },
	{ "ObjCrLaser_GetNodeColor", StgStageScript::Func_ObjCrLaser_GetNodeColor, 2 },
	{ "ObjCrLaser_SetNode", StgStageScript::Func_ObjCrLaser_SetNode, 6 },
	{ "ObjCrLaser_SetNodePosition", StgStageScript::Func_ObjCrLaser_SetNodePosition, 4 },
	{ "ObjCrLaser_SetNodeAngle", StgStageScript::Func_ObjCrLaser_SetNodeAngle, 3 },
	{ "ObjCrLaser_SetNodeColor", StgStageScript::Func_ObjCrLaser_SetNodeColor, 3 },
	{ "ObjCrLaser_AddNode", StgStageScript::Func_ObjCrLaser_AddNode, 5 },

	{ "ObjPatternShot_Create", StgStageScript::Func_ObjPatternShot_Create, 0 },
	{ "ObjPatternShot_Fire", StgStageScript::Func_ObjPatternShot_Fire, 1 },
	{ "ObjPatternShot_FireReturn", StgStageScript::Func_ObjPatternShot_FireReturn, 1 },
	{ "ObjPatternShot_SetParentObject", StgStageScript::Func_ObjPatternShot_SetParentObject, 2 },
	{ "ObjPatternShot_SetPatternType", StgStageScript::Func_ObjPatternShot_SetPatternType, 2 },
	{ "ObjPatternShot_SetShotType", StgStageScript::Func_ObjPatternShot_SetShotType, 2 },
	{ "ObjPatternShot_SetInitialBlendMode", StgStageScript::Func_ObjPatternShot_SetInitialBlendMode, 2 },
	{ "ObjPatternShot_SetShotCount", StgStageScript::Func_ObjPatternShot_SetShotCount, 3 },
	{ "ObjPatternShot_SetSpeed", StgStageScript::Func_ObjPatternShot_SetSpeed, 3 },
	{ "ObjPatternShot_SetAngle", StgStageScript::Func_ObjPatternShot_SetAngle, 3 },
	{ "ObjPatternShot_SetBasePoint", StgStageScript::Func_ObjPatternShot_SetBasePoint, 3 },
	{ "ObjPatternShot_SetBasePointOffset", StgStageScript::Func_ObjPatternShot_SetBasePointOffset, 3 },
	{ "ObjPatternShot_SetBasePointOffsetCircle", StgStageScript::Func_ObjPatternShot_SetBasePointOffsetCircle, 3 },
	{ "ObjPatternShot_SetShootRadius", StgStageScript::Func_ObjPatternShot_SetShootRadius, 2 },
	{ "ObjPatternShot_SetDelay", StgStageScript::Func_ObjPatternShot_SetDelay, 2 },
	//{ "ObjPatternShot_SetDelayMotion", StgStageScript::Func_ObjPatternShot_SetDelayMotion, 2 },
	{ "ObjPatternShot_SetGraphic", StgStageScript::Func_ObjPatternShot_SetGraphic, 2 },
	{ "ObjPatternShot_SetLaserParameter", StgStageScript::Func_ObjPatternShot_SetLaserParameter, 3 },
	{ "ObjPatternShot_CopySettings", StgStageScript::Func_ObjPatternShot_CopySettings, 2 },
	{ "ObjPatternShot_AddTransform", StgStageScript::Func_ObjPatternShot_AddTransform, 8 },
	{ "ObjPatternShot_SetTransform", StgStageScript::Func_ObjPatternShot_SetTransform, 9 },

	//STG共通関数：アイテムオブジェクト操作
	{ "ObjItem_Create", StgStageScript::Func_ObjItem_Create, 1 },
	{ "ObjItem_Regist", StgStageScript::Func_ObjItem_Regist, 1 },
	{ "ObjItem_SetItemID", StgStageScript::Func_ObjItem_SetItemID, 2 },
	{ "ObjItem_SetRenderScoreEnable", StgStageScript::Func_ObjItem_SetRenderScoreEnable, 2 },
	{ "ObjItem_SetAutoCollectEnable", StgStageScript::Func_ObjItem_SetAutoCollectEnable, 2 },
	{ "ObjItem_SetDefinedMovePatternA1", StgStageScript::Func_ObjItem_SetDefinedMovePatternA1, 2 },

		//STG共通関数：自機オブジェクト操作
	{ "ObjPlayer_AddIntersectionCircleA1", StgStageScript::Func_ObjPlayer_AddIntersectionCircleA1, 5 },
	{ "ObjPlayer_AddIntersectionCircleA2", StgStageScript::Func_ObjPlayer_AddIntersectionCircleA2, 4 },
	{ "ObjPlayer_ClearIntersection", StgStageScript::Func_ObjPlayer_ClearIntersection, 1 },

		//STG共通関数：当たり判定オブジェクト操作
	{ "ObjCol_IsIntersected", StgStageScript::Func_ObjCol_IsIntersected, 1 },
	{ "ObjCol_GetListOfIntersectedEnemyID", StgStageScript::Func_ObjCol_GetListOfIntersectedEnemyID, 1 },
	{ "ObjCol_GetListOfIntersectedShotID", StgStageScript::Func_ObjCol_GetListOfIntersectedShotID, 1 },
	{ "ObjCol_GetIntersectedCount", StgStageScript::Func_ObjCol_GetIntersectedCount, 1 },

	//定数
	{ "SCREEN_WIDTH", constant<640>::func, 0 },
	{ "SCREEN_HEIGHT", constant<480>::func, 0 },
	{ "TYPE_ALL", constant<StgStageScript::TYPE_ALL>::func, 0 },
	{ "TYPE_SHOT", constant<StgStageScript::TYPE_SHOT>::func, 0 },
	{ "TYPE_CHILD", constant<StgStageScript::TYPE_CHILD>::func, 0 },
	{ "TYPE_IMMEDIATE", constant<StgStageScript::TYPE_IMMEDIATE>::func, 0 },
	{ "TYPE_FADE", constant<StgStageScript::TYPE_FADE>::func, 0 },
	{ "TYPE_ITEM", constant<StgStageScript::TYPE_ITEM>::func, 0 },

	{ "OWNER_PLAYER", constant<StgShotObject::OWNER_PLAYER>::func, 0 },
	{ "OWNER_ENEMY", constant<StgShotObject::OWNER_ENEMY>::func, 0 },

	{ "DELAY_DEFAULT", constant<StgShotObject::DelayParameter::DELAY_DEFAULT>::func, 0 },
	{ "DELAY_LERP", constant<StgShotObject::DelayParameter::DELAY_LERP>::func, 0 },

	{ "PATTERN_FAN", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_FAN>::func, 0 },
	{ "PATTERN_FAN_AIMED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_FAN_AIMED>::func, 0 },
	{ "PATTERN_RING", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_RING>::func, 0 },
	{ "PATTERN_RING_AIMED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_RING_AIMED>::func, 0 },
	{ "PATTERN_ARROW", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_ARROW>::func, 0 },
	{ "PATTERN_ARROW_AIMED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_ARROW_AIMED>::func, 0 },
	{ "PATTERN_POLYGON", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_POLYGON>::func, 0 },
	{ "PATTERN_POLYGON_AIMED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_POLYGON_AIMED>::func, 0 },
	{ "PATTERN_ELLIPSE", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_ELLIPSE>::func, 0 },
	{ "PATTERN_ELLIPSE_AIMED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_ELLIPSE_AIMED>::func, 0 },
	{ "PATTERN_SCATTER_ANGLE", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_SCATTER_ANGLE>::func, 0 },
	{ "PATTERN_SCATTER_SPEED", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_SCATTER_SPEED>::func, 0 },
	{ "PATTERN_SCATTER", constant<StgPatternShotObjectGenerator::PATTERN_TYPE_SCATTER>::func, 0 },
	{ "PATTERN_BASEPOINT_RESET", constant<StgPatternShotObjectGenerator::BASEPOINT_RESET>::func, 0 },

	{ "TRANSFORM_WAIT", constant<StgPatternShotTransform::TRANSFORM_WAIT>::func, 0 },
	{ "TRANSFORM_ADD_SPEED_ANGLE", constant<StgPatternShotTransform::TRANSFORM_ADD_SPEED_ANGLE>::func, 0 },
	{ "TRANSFORM_ANGULAR_MOVE", constant<StgPatternShotTransform::TRANSFORM_ANGULAR_MOVE>::func, 0 },
	{ "TRANSFORM_N_DECEL_CHANGE", constant<StgPatternShotTransform::TRANSFORM_N_DECEL_CHANGE>::func, 0 },
	{ "TRANSFORM_GRAPHIC_CHANGE", constant<StgPatternShotTransform::TRANSFORM_GRAPHIC_CHANGE>::func, 0 },
	{ "TRANSFORM_BLEND_CHANGE", constant<StgPatternShotTransform::TRANSFORM_BLEND_CHANGE>::func, 0 },
	{ "TRANSFORM_TO_SPEED_ANGLE", constant<StgPatternShotTransform::TRANSFORM_TO_SPEED_ANGLE>::func, 0 },
	{ "TRANSFORM_ADDPATTERNA1", constant<StgPatternShotTransform::TRANSFORM_ADDPATTERNA1>::func, 0 },
	{ "TRANSFORM_ADDPATTERNA2", constant<StgPatternShotTransform::TRANSFORM_ADDPATTERNA2>::func, 0 },

	{ "STATE_NORMAL", constant<StgPlayerObject::STATE_NORMAL>::func, 0 },
	{ "STATE_HIT", constant<StgPlayerObject::STATE_HIT>::func, 0 },
	{ "STATE_DOWN", constant<StgPlayerObject::STATE_DOWN>::func, 0 },
	{ "STATE_END", constant<StgPlayerObject::STATE_END>::func, 0 },

	{ "ITEM_1UP", constant<StgItemObject::ITEM_1UP>::func, 0 },
	{ "ITEM_1UP_S", constant<StgItemObject::ITEM_1UP_S>::func, 0 },
	{ "ITEM_SPELL", constant<StgItemObject::ITEM_SPELL>::func, 0 },
	{ "ITEM_SPELL_S", constant<StgItemObject::ITEM_SPELL_S>::func, 0 },
	{ "ITEM_POWER", constant<StgItemObject::ITEM_POWER>::func, 0 },
	{ "ITEM_POWER_S", constant<StgItemObject::ITEM_POWER_S>::func, 0 },
	{ "ITEM_POINT", constant<StgItemObject::ITEM_POINT>::func, 0 },
	{ "ITEM_POINT_S", constant<StgItemObject::ITEM_POINT_S>::func, 0 },
	{ "ITEM_USER", constant<StgItemObject::ITEM_USER>::func, 0 },

	{ "ITEM_MOVE_DOWN", constant<StgMovePattern_Item::MOVE_DOWN>::func, 0 },
	{ "ITEM_MOVE_TOPLAYER", constant<StgMovePattern_Item::MOVE_TOPLAYER>::func, 0 },
	{ "ITEM_MOVE_SCORE", constant<StgMovePattern_Item::MOVE_SCORE>::func, 0 },

	{ "OBJ_PLAYER", constant<(int)TypeObject::OBJ_PLAYER>::func, 0 },
	{ "OBJ_SPELL_MANAGE", constant<(int)TypeObject::OBJ_SPELL_MANAGE>::func, 0 },
	{ "OBJ_SPELL", constant<(int)TypeObject::OBJ_SPELL>::func, 0 },
	{ "OBJ_ENEMY", constant<(int)TypeObject::OBJ_ENEMY>::func, 0 },
	{ "OBJ_ENEMY_BOSS", constant<(int)TypeObject::OBJ_ENEMY_BOSS>::func, 0 },
	{ "OBJ_ENEMY_BOSS_SCENE", constant<(int)TypeObject::OBJ_ENEMY_BOSS_SCENE>::func, 0 },
	{ "OBJ_SHOT", constant<(int)TypeObject::OBJ_SHOT>::func, 0 },
	{ "OBJ_LOOSE_LASER", constant<(int)TypeObject::OBJ_LOOSE_LASER>::func, 0 },
	{ "OBJ_STRAIGHT_LASER", constant<(int)TypeObject::OBJ_STRAIGHT_LASER>::func, 0 },
	{ "OBJ_CURVE_LASER", constant<(int)TypeObject::OBJ_CURVE_LASER>::func, 0 },
	{ "OBJ_ITEM", constant<(int)TypeObject::OBJ_ITEM>::func, 0 },

	{ "INFO_LIFE", constant<StgStageScript::INFO_LIFE>::func, 0 },
	{ "INFO_DAMAGE_RATE_SHOT", constant<StgStageScript::INFO_DAMAGE_RATE_SHOT>::func, 0 },
	{ "INFO_DAMAGE_RATE_SPELL", constant<StgStageScript::INFO_DAMAGE_RATE_SPELL>::func, 0 },
	{ "INFO_SHOT_HIT_COUNT", constant<StgStageScript::INFO_SHOT_HIT_COUNT>::func, 0 },
	{ "INFO_TIMER", constant<StgStageScript::INFO_TIMER>::func, 0 },
	{ "INFO_TIMERF", constant<StgStageScript::INFO_TIMERF>::func, 0 },
	{ "INFO_ORGTIMERF", constant<StgStageScript::INFO_ORGTIMERF>::func, 0 },
	{ "INFO_IS_SPELL", constant<StgStageScript::INFO_IS_SPELL>::func, 0 },
	{ "INFO_IS_LAST_SPELL", constant<StgStageScript::INFO_IS_LAST_SPELL>::func, 0 },
	{ "INFO_IS_DURABLE_SPELL", constant<StgStageScript::INFO_IS_DURABLE_SPELL>::func, 0 },
	{ "INFO_SPELL_SCORE", constant<StgStageScript::INFO_SPELL_SCORE>::func, 0 },
	{ "INFO_REMAIN_STEP_COUNT", constant<StgStageScript::INFO_REMAIN_STEP_COUNT>::func, 0 },
	{ "INFO_ACTIVE_STEP_LIFE_COUNT", constant<StgStageScript::INFO_ACTIVE_STEP_LIFE_COUNT>::func, 0 },
	{ "INFO_ACTIVE_STEP_TOTAL_MAX_LIFE", constant<StgStageScript::INFO_ACTIVE_STEP_TOTAL_MAX_LIFE>::func, 0 },
	{ "INFO_ACTIVE_STEP_TOTAL_LIFE", constant<StgStageScript::INFO_ACTIVE_STEP_TOTAL_LIFE>::func, 0 },
	{ "INFO_ACTIVE_STEP_LIFE_RATE_LIST", constant<StgStageScript::INFO_ACTIVE_STEP_LIFE_RATE_LIST>::func, 0 },
	{ "INFO_IS_LAST_STEP", constant<StgStageScript::INFO_IS_LAST_STEP>::func, 0 },
	{ "INFO_PLAYER_SHOOTDOWN_COUNT", constant<StgStageScript::INFO_PLAYER_SHOOTDOWN_COUNT>::func, 0 },
	{ "INFO_PLAYER_SPELL_COUNT", constant<StgStageScript::INFO_PLAYER_SPELL_COUNT>::func, 0 },
	{ "INFO_CURRENT_LIFE", constant<StgStageScript::INFO_CURRENT_LIFE>::func, 0 },
	{ "INFO_CURRENT_LIFE_MAX", constant<StgStageScript::INFO_CURRENT_LIFE_MAX>::func, 0 },

	{ "INFO_ITEM_SCORE", constant<StgStageScript::INFO_ITEM_SCORE>::func, 0 },

	{ "INFO_EXISTS", constant<StgStageScript::INFO_EXISTS>::func, 0 },
	{ "INFO_PATH", constant<StgStageScript::INFO_PATH>::func, 0 },
	{ "INFO_RECT", constant<StgStageScript::INFO_RECT>::func, 0 },
	{ "INFO_DELAY_COLOR", constant<StgStageScript::INFO_DELAY_COLOR>::func, 0 },
	{ "INFO_BLEND", constant<StgStageScript::INFO_BLEND>::func, 0 },
	{ "INFO_COLLISION", constant<StgStageScript::INFO_COLLISION>::func, 0 },
	{ "INFO_COLLISION_LIST", constant<StgStageScript::INFO_COLLISION_LIST>::func, 0 },
	{ "INFO_IS_FIXED_ANGLE", constant<StgStageScript::INFO_IS_FIXED_ANGLE>::func, 0 },

	{ "EV_REQUEST_LIFE", constant<StgStageScript::EV_REQUEST_LIFE>::func, 0 },
	{ "EV_REQUEST_TIMER", constant<StgStageScript::EV_REQUEST_TIMER>::func, 0 },
	{ "EV_REQUEST_IS_SPELL", constant<StgStageScript::EV_REQUEST_IS_SPELL>::func, 0 },
	{ "EV_REQUEST_IS_LAST_SPELL", constant<StgStageScript::EV_REQUEST_IS_LAST_SPELL>::func, 0 },
	{ "EV_REQUEST_IS_DURABLE_SPELL", constant<StgStageScript::EV_REQUEST_IS_DURABLE_SPELL>::func, 0 },
	{ "EV_REQUEST_REQUIRE_ALL_DOWN", constant<StgStageScript::EV_REQUEST_REQUIRE_ALL_DOWN>::func, 0 },
	{ "EV_REQUEST_SPELL_SCORE", constant<StgStageScript::EV_REQUEST_SPELL_SCORE>::func, 0 },
	{ "EV_REQUEST_REPLAY_TARGET_COMMON_AREA", constant<StgStageScript::EV_REQUEST_REPLAY_TARGET_COMMON_AREA>::func, 0 },

	{ "EV_TIMEOUT", constant<StgStageScript::EV_TIMEOUT>::func, 0 },
	{ "EV_START_BOSS_SPELL", constant<StgStageScript::EV_START_BOSS_SPELL>::func, 0 },
	{ "EV_END_BOSS_SPELL", constant<StgStageScript::EV_END_BOSS_SPELL>::func, 0 },
	{ "EV_GAIN_SPELL", constant<StgStageScript::EV_GAIN_SPELL>::func, 0 },
	{ "EV_START_BOSS_STEP", constant<StgStageScript::EV_START_BOSS_STEP>::func, 0 },
	{ "EV_END_BOSS_STEP", constant<StgStageScript::EV_END_BOSS_STEP>::func, 0 },

	{ "EV_PLAYER_SHOOTDOWN", constant<StgStageScript::EV_PLAYER_SHOOTDOWN>::func, 0 },
	{ "EV_PLAYER_SPELL", constant<StgStageScript::EV_PLAYER_SPELL>::func, 0 },
	{ "EV_PLAYER_REBIRTH", constant<StgStageScript::EV_PLAYER_REBIRTH>::func, 0 },

	{ "REBIRTH_DEFAULT", constant<StgPlayerObject::REBIRTH_DEFAULT>::func, 0 },

	{ "EV_PAUSE_ENTER", constant<StgStageScript::EV_PAUSE_ENTER>::func, 0 },
	{ "EV_PAUSE_LEAVE", constant<StgStageScript::EV_PAUSE_LEAVE>::func, 0 },

	{ "TARGET_ALL", constant<StgStageScript::TARGET_ALL>::func, 0 },
	{ "TARGET_ENEMY", constant<StgStageScript::TARGET_ENEMY>::func, 0 },
	{ "TARGET_PLAYER", constant<StgStageScript::TARGET_PLAYER>::func, 0 },

	{ "TOPLAYER_CHANGE", constant<StgMovePattern::TOPLAYER_CHANGE>::func, 0 },
	{ "NO_CHANGE", constant<StgMovePattern::NO_CHANGE>::func, 0 },
};
StgStageScript::StgStageScript(StgStageController* stageController) : StgControlScript(stageController->GetSystemController()) {
	stageController_ = stageController;

	typeScript_ = TYPE_STAGE;
	_AddFunction(stgFunction, sizeof(stgFunction) / sizeof(function));

	ref_count_ptr<StgStageInformation> info = stageController_->GetStageInformation();
	mt_ = info->GetRandProvider();

	scriptManager_ = stageController_->GetScriptManager();
	StgStageScriptManager* scriptManager = (StgStageScriptManager*)scriptManager_;
	SetObjectManager(scriptManager->GetObjectManager());
}
StgStageScript::~StgStageScript() {}
std::shared_ptr<StgStageScriptObjectManager> StgStageScript::GetStgObjectManager() {
	StgStageScriptManager* scriptManager = (StgStageScriptManager*)scriptManager_;
	return scriptManager->GetObjectManager();
}


//STG制御共通関数：共通データ
gstd::value StgStageScript::Func_SaveCommonDataAreaToReplayFile(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	if (infoStage->IsReplay())
		script->RaiseError(L"This function can only be called outside of replays.");

	bool res = false;
	std::string area = StringUtility::ConvertWideToMulti(argv[0].as_string());

	shared_ptr<ScriptCommonData> commonDataO = commonDataManager->GetData(area);
	if (commonDataO) {
		shared_ptr<ScriptCommonData> commonDataS(new ScriptCommonData());
		commonDataS->Copy(commonDataO);
		replayStageData->SetCommonData(area, commonDataS);
		res = true;
	}
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_LoadCommonDataAreaFromReplayFile(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	if (!infoStage->IsReplay())
		script->RaiseError(L"This function can only be called during replays.");

	bool res = false;
	std::string area = StringUtility::ConvertWideToMulti(argv[0].as_string());

	shared_ptr<ScriptCommonData> commonDataS = replayStageData->GetCommonData(area);
	if (commonDataS) {
		shared_ptr<ScriptCommonData> commonDataO(new ScriptCommonData());
		commonDataO->Copy(commonDataS);
		commonDataManager->SetData(area, commonDataO);
		res = true;
	}
	return script->CreateBooleanValue(res);
}

//STG共通関数：システム関連
gstd::value StgStageScript::Func_GetMainStgScriptPath(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<ScriptInformation> infoMain = stageController->GetStageInformation()->GetMainScriptInformation();

	std::wstring path = infoMain->GetScriptPath();
	path = PathProperty::GetUnique(path);

	return script->CreateStringValue(path);
}
gstd::value StgStageScript::Func_GetMainStgScriptDirectory(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<ScriptInformation> infoMain = stageController->GetStageInformation()->GetMainScriptInformation();

	std::wstring path = infoMain->GetScriptPath();
	path = PathProperty::GetUnique(path);

	std::wstring dir = PathProperty::GetFileDirectory(path);

	return script->CreateStringValue(dir);
}
gstd::value StgStageScript::Func_SetStgFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	RECT rect;
	rect.left = (LONG)argv[0].as_int();
	rect.top = (LONG)argv[1].as_int();
	rect.right = (LONG)argv[2].as_int();
	rect.bottom = (LONG)argv[3].as_int();

	int min = argv[4].as_int();
	int max = argv[5].as_int();

	ref_count_ptr<StgStageInformation> stageInfo = stageController->GetStageInformation();
	stageInfo->SetStgFrameRect(rect);
	stageInfo->SetStgFrameMinPriority(min);
	stageInfo->SetStgFrameMaxPriority(max);

	return value();
}

gstd::value StgStageScript::Func_SetItemRenderPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	int pri = argv[0].as_int();
	//pri = min(pri, info->GetStgFrameMaxPriority());
	//pri = max(pri, info->GetStgFrameMinPriority());
	info->SetItemObjectPriority(pri);
	return value();
}
gstd::value StgStageScript::Func_SetShotRenderPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	int pri = argv[0].as_int();
	//pri = min(pri, info->GetStgFrameMaxPriority());
	//pri = max(pri, info->GetStgFrameMinPriority());
	info->SetShotObjectPriority(pri);
	return value();
}
gstd::value StgStageScript::Func_GetStgFrameRenderPriorityMinI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int res = stageController->GetStageInformation()->GetStgFrameMinPriority();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetStgFrameRenderPriorityMaxI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int res = stageController->GetStageInformation()->GetStgFrameMaxPriority();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetItemRenderPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int res = stageController->GetStageInformation()->GetItemObjectPriority();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetShotRenderPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int res = stageController->GetStageInformation()->GetShotObjectPriority();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetPlayerRenderPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	auto objectManager = script->GetStgObjectManager();
	int idObjPlayer = objectManager->GetPlayerObjectID();

	int res = 30;
	StgPlayerObject* obj = dynamic_cast<StgPlayerObject*>(script->GetObjectPointer(idObjPlayer));
	if (obj)
		res = obj->GetRenderPriorityI();

	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetCameraFocusPermitPriorityI(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int res = stageController->GetStageInformation()->GetCameraFocusPermitPriority();
	return script->CreateRealValue(res);
}

gstd::value StgStageScript::Func_CloseStgScene(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgSystemController* systemController = script->stageController_->GetSystemController();

	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	info->SetEnd();

	return value();
}
gstd::value StgStageScript::Func_GetReplayFps(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	int fps = 0;
	if (infoStage->IsReplay()) {
		ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
		int frame = infoStage->GetCurrentFrame();
		fps = replayStageData->GetFramePerSecond(frame);
	}

	return script->CreateRealValue(fps);
}

//STG共通関数：自機
gstd::value StgStageScript::Func_GetPlayerObjectID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	auto objectManager = script->GetStgObjectManager();
	int res = objectManager->GetPlayerObjectID();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetPlayerScriptID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	auto scriptManager = script->stageController_->GetScriptManager();
	int64_t res = scriptManager->GetPlayerScriptID();
	return script->CreateRealValue((double)res);
}
gstd::value StgStageScript::Func_SetPlayerSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double speedFast = argv[0].as_real();
		double speedSlow = argv[1].as_real();
		obj->SetFastSpeed(speedFast);
		obj->SetSlowSpeed(speedSlow);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerClip(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		RECT rect;
		rect.left = (LONG)argv[0].as_int();
		rect.top = (LONG)argv[1].as_int();
		rect.right = (LONG)argv[2].as_int();
		rect.bottom = (LONG)argv[3].as_int();
		obj->SetClip(rect);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerLife(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double life = argv[0].as_real();
		obj->SetLife(life);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerSpell(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double spell = argv[0].as_real();
		obj->SetSpell(spell);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerPower(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double power = argv[0].as_real();
		obj->SetPower(power);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerInvincibilityFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		int invi = argv[0].as_int();
		obj->SetInvincibilityFrame(invi);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerDownStateFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		int frame = argv[0].as_int();
		obj->SetDownStateFrame(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerRebirthFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		int frame = argv[0].as_int();
		obj->SetRebirthFrame(frame);
		obj->SetRebirthFrameMax(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerRebirthLossFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		int frame = argv[0].as_int();
		obj->SetRebirthLossFrame(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerAutoItemCollectLine(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		int posY = argv[0].as_int();
		obj->SetAutoItemCollectY(posY);
	}
	return value();
}
gstd::value StgStageScript::Func_SetPlayerItemScope(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double radius = argv[0].as_real();
		obj->SetItemIntersectionRadius(radius);
	}
	return value();
}
template<void (StgPlayerObject::*Func)(bool)>
gstd::value StgStageScript::Func_SetPlayerInfoAsBool(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) 
		(obj.get()->*Func)(argv[0].as_boolean());
	return value();
}
template<double (StgPlayerObject::*Func)(void), int DEF>
gstd::value StgStageScript::Func_GetPlayerInfoAsDbl(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	double res = obj ? (obj.get()->*Func)() : (double)DEF;
	return script->CreateRealValue(res);
}
template<int (StgPlayerObject::*Func)(void), int DEF>
gstd::value StgStageScript::Func_GetPlayerInfoAsInt(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	int res = obj ? (obj.get()->*Func)() : DEF;
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_SetPlayerRebirthPosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		double x = argv[0].as_real();
		double y = argv[1].as_real();
		obj->SetRebirthPosition(x, y);
	}
	return value();
}
gstd::value StgStageScript::Func_GetPlayerSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	double listValue[2] = { 0, 0 };
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		listValue[0] = obj->GetFastSpeed();
		listValue[1] = obj->GetSlowSpeed();
	}
	return script->CreateRealArrayValue(listValue, 2U);
}
gstd::value StgStageScript::Func_GetPlayerClip(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	LONG listValue[4] = { 0, 0, 0, 0 };
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	if (obj) {
		RECT* clip = obj->GetClip();
		memcpy(listValue, clip, sizeof(LONG) * 4U);
	}
	return script->CreateRealArrayValue(listValue, 4);
}
gstd::value StgStageScript::Func_GetAngleToPlayer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	double angle = 0;
	shared_ptr<StgPlayerObject> objPlayer = script->stageController_->GetPlayerObject();
	if (objPlayer) {
		double px = objPlayer->GetPositionX();
		double py = objPlayer->GetPositionY();

		int id = (int)argv[0].as_real();
		shared_ptr<DxScriptRenderObject> objMove = std::dynamic_pointer_cast<DxScriptRenderObject>(script->GetObject(id));
		if (objMove) {
			double tx = objMove->GetPosition().x;
			double ty = objMove->GetPosition().y;
			angle = Math::RadianToDegree(atan2(py - ty, px - tx));
		}
	}
	return script->CreateRealValue(angle);
}

gstd::value StgStageScript::Func_IsPermitPlayerShot(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	bool res = obj ? obj->IsPermitShot() : false;
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_IsPermitPlayerSpell(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	bool res = obj ? obj->IsPermitSpell() : false;
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_IsPlayerLastSpellWait(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	shared_ptr<StgPlayerObject> obj = script->stageController_->GetPlayerObject();
	bool res = obj ? obj->IsWaitLastSpell() : false;
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_IsPlayerSpellActive(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	bool res = false;
	shared_ptr<StgPlayerObject> objPlayer = script->stageController_->GetPlayerObject();
	if (objPlayer) {
		shared_ptr<StgPlayerSpellManageObject> objSpell = objPlayer->GetSpellManageObject();
		res = objSpell != nullptr && !objSpell->IsDeleted();
	}
	return script->CreateBooleanValue(res);
}


//STG共通関数：敵
gstd::value StgStageScript::Func_GetEnemyBossSceneObjectID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	int res = ID_INVALID;
	shared_ptr<StgEnemyBossSceneObject> obj = enemyManager->GetBossSceneObject();
	if (obj != nullptr && !obj->IsDeleted())
		res = obj->GetObjectID();

	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_GetEnemyBossObjectID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();
	shared_ptr<StgEnemyBossSceneObject> scene = enemyManager->GetBossSceneObject();

	std::vector<int> listID;
	if (scene) {
		shared_ptr<StgEnemyBossSceneData> data = scene->GetActiveData();
		if (data) {
			for (auto& iEnemy : data->GetEnemyObjectList()) {
				if (iEnemy->IsDeleted()) continue;
				listID.push_back(iEnemy->GetObjectID());
			}
		}
	}

	return script->CreateRealArrayValue(listID);
}
gstd::value StgStageScript::Func_GetAllEnemyID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	std::vector<int> listID;
	for (auto& iEnemy : enemyManager->GetEnemyList()) {
		if (iEnemy->IsDeleted()) continue;
		listID.push_back(iEnemy->GetObjectID());
	}

	return script->CreateRealArrayValue(listID);
}
gstd::value StgStageScript::Func_GetIntersectionRegistedEnemyID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	std::vector<int> listID;
	std::vector<StgIntersectionTargetPoint>* listPoint = intersectionManager->GetAllEnemyTargetPoint();
	for (auto itr = listPoint->begin(); itr != listPoint->end(); ++itr) {
		if (auto ptrObj = itr->GetObjectRef().lock())
			listID.push_back(ptrObj->GetObjectID());
	}

	return script->CreateRealArrayValue(listID);
}
gstd::value StgStageScript::Func_GetAllEnemyIntersectionPosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();

	std::vector<gstd::value> listV;
	std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
	for (auto itr = listPoint->begin(); itr != listPoint->end(); ++itr) {
		if (auto ptrObj = itr->GetObjectRef().lock()) {
			if (!ptrObj->GetEnableGetIntersectionPosition()) continue;

			POINT& pos = itr->GetPoint();
			LONG listPos[2] = { pos.x, pos.y };
			gstd::value v = script->CreateRealArrayValue(listPos, 2U);
			listV.push_back(v);
		}
	}
	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int cenX = argv[0].as_int();
	int cenY = argv[1].as_int();
	int countRes = argv[2].as_int();

	std::map<int64_t, POINT> mapPos;
	std::vector<gstd::value> listV;

	std::vector<StgIntersectionTargetPoint>* listPoint = intersectionManager->GetAllEnemyTargetPoint();
	for (auto itr = listPoint->begin(); itr != listPoint->end(); ++itr) {
		if (auto ptrObj = itr->GetObjectRef().lock()) {
			if (!ptrObj->GetEnableGetIntersectionPosition()) continue;

			POINT& pos = itr->GetPoint();
			LONG dx = pos.x - cenX;
			LONG dy = pos.y - cenY;

			int64_t dist = dx * dx + dy * dy;
			mapPos[dist] = pos;
		}
	}

	for (auto itr = mapPos.begin(); (itr != mapPos.end()) && (countRes > 0); ++itr) {
		POINT& pos = itr->second;
		LONG listPosRes[2] = { pos.x, pos.y };
		gstd::value v = script->CreateRealArrayValue(listPosRes, 2U);
		listV.push_back(v);
		--countRes;
	}

	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPositionByIdA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	//引数1（敵オブジェクトID）自機からもアクセス可能
	//指定した敵オブジェクトIDが持つ自機ショットへの当たり判定位置を全て取得
	//二次元配列が返る。([<インデックス>][<0:x座標, 1:y座標>])　配列の0番目が最も敵本体の座標に近い
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));

	std::vector<gstd::value> listV;
	if (obj) {
		std::map<int64_t, POINT> mapPos;
		int enemyX = obj->GetPositionX();
		int enemyY = obj->GetPositionY();

		StgStageController* stageController = script->stageController_;
		StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();

		std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
		for (auto itr = listPoint->begin(); itr != listPoint->end(); ++itr) {
			if (auto ptrObj = itr->GetObjectRef().lock()) {
				if (!ptrObj->GetEnableGetIntersectionPosition()) continue;
				if (ptrObj->GetObjectID() != id) continue;

				POINT& pos = itr->GetPoint();
				LONG dx = pos.x - enemyX;
				LONG dy = pos.y - enemyY;

				int64_t dist = dx * dx + dy * dy;
				mapPos[dist] = pos;
			}
		}

		for (auto itr = mapPos.begin(); itr != mapPos.end(); ++itr) {
			POINT& pos = itr->second;
			LONG listPos[2] = { pos.x, pos.y };
			gstd::value v = script->CreateRealArrayValue(listPos, 2U);
			listV.push_back(v);
		}
	}

	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPositionByIdA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	//引数3（敵オブジェクトID・x座標・y座標）自機からもアクセス可能
	//指定した敵オブジェクトIDが持つ、自機ショットへの当たり判定のうち、指定座標に最も近い1つを取得
	//配列が返る。([<0:x座標, 1:y座標>])

	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));

	std::vector<gstd::value> listV;
	if (obj) {
		std::map<int64_t, POINT> mapPos;
		int tX = argv[1].as_int();
		int tY = argv[2].as_int();

		StgStageController* stageController = script->stageController_;
		StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();

		std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
		for (auto itr = listPoint->begin(); itr != listPoint->end(); ++itr) {
			if (auto ptrObj = itr->GetObjectRef().lock()) {
				if (!ptrObj->GetEnableGetIntersectionPosition()) continue;
				if (ptrObj->GetObjectID() != id) continue;

				POINT& pos = itr->GetPoint();
				LONG dx = pos.x - tX;
				LONG dy = pos.y - tY;

				int64_t dist = dx * dx + dy * dy;
				mapPos[dist] = pos;
			}
		}

		for (auto itr = mapPos.begin(); itr != mapPos.end(); ++itr) {
			POINT& pos = itr->second;
			LONG listPos[2] = { pos.x, pos.y };
			gstd::value v = script->CreateRealArrayValue(listPos, 2U);
			listV.push_back(v);
		}
	}

	return script->CreateValueArrayValue(listV);
}

gstd::value StgStageScript::Func_LoadEnemyShotData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadEnemyShotData(path);

	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_ReloadEnemyShotData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadEnemyShotData(path, true);

	return script->CreateBooleanValue(res);
}

//STG共通関数：弾
gstd::value StgStageScript::Func_DeleteShotAll(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeDel = argv[0].as_int();
	int typeTo = argv[1].as_int();

	switch (typeDel) {
	case TYPE_ALL:typeDel = StgShotManager::DEL_TYPE_ALL; break;
	case TYPE_SHOT:typeDel = StgShotManager::DEL_TYPE_SHOT; break;
	case TYPE_CHILD:typeDel = StgShotManager::DEL_TYPE_CHILD; break;
	}

	switch (typeTo) {
	case TYPE_IMMEDIATE:typeTo = StgShotManager::TO_TYPE_IMMEDIATE; break;
	case TYPE_FADE:typeTo = StgShotManager::TO_TYPE_FADE; break;
	case TYPE_ITEM:typeTo = StgShotManager::TO_TYPE_ITEM; break;
	}

	stageController->GetShotManager()->DeleteInCircle(typeDel, typeTo, StgShotObject::OWNER_ENEMY, 0, 0, 256 * 256);

	return value();
}
gstd::value StgStageScript::Func_DeleteShotInCircle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeDel = argv[0].as_int();
	int typeTo = argv[1].as_int();
	float posX = argv[2].as_real();
	float posY = argv[3].as_real();
	float radius = argv[4].as_real();

	switch (typeDel) {
	case TYPE_ALL:typeDel = StgShotManager::DEL_TYPE_ALL; break;
	case TYPE_SHOT:typeDel = StgShotManager::DEL_TYPE_SHOT; break;
	case TYPE_CHILD:typeDel = StgShotManager::DEL_TYPE_CHILD; break;
	}

	switch (typeTo) {
	case TYPE_IMMEDIATE:typeTo = StgShotManager::TO_TYPE_IMMEDIATE; break;
	case TYPE_FADE:typeTo = StgShotManager::TO_TYPE_FADE; break;
	case TYPE_ITEM:typeTo = StgShotManager::TO_TYPE_ITEM; break;
	}

	stageController->GetShotManager()->DeleteInCircle(typeDel, typeTo, StgShotObject::OWNER_ENEMY, posX, posY, radius);

	return value();
}
gstd::value StgStageScript::Func_CreateShotA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speed = argv[2].as_real();
			double angle = argv[3].as_real();
			int idShot = argv[4].as_int();
			int delay = argv[5].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetSpeed(speed);
			obj->SetDirectionAngle(Math::DegreeToRadian(angle));
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateShotA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speed = argv[2].as_real();
			double angle = argv[3].as_real();
			double accele = argv[4].as_real();
			double maxSpeed = argv[5].as_real();
			int idShot = argv[6].as_int();
			int delay = argv[7].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetSpeed(speed);
			obj->SetDirectionAngle(Math::DegreeToRadian(angle));
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);

			StgMoveObject* objMove = (StgMoveObject*)obj.get();
			StgMovePattern_Angle* pattern = dynamic_cast<StgMovePattern_Angle*>(objMove->GetPattern().get());
			pattern->SetAcceleration(accele);
			pattern->SetMaxSpeed(maxSpeed);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateShotOA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		int tId = (int)argv[0].as_real();
		DxScriptRenderObject* tObj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(tId));
		if (tObj) {
			double posX = tObj->GetPosition().x;
			double posY = tObj->GetPosition().y;

			shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
			id = script->AddObject(obj);
			if (id != ID_INVALID) {
				stageController->GetShotManager()->AddShot(obj);
				double speed = argv[1].as_real();
				double angle = argv[2].as_real();
				int idShot = argv[3].as_int();
				int delay = argv[4].as_int();
				int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
					StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

				obj->SetX(posX);
				obj->SetY(posY);
				obj->SetSpeed(speed);
				obj->SetDirectionAngle(Math::DegreeToRadian(angle));
				obj->SetShotDataID(idShot);
				obj->SetDelay(delay);
				obj->SetOwnerType(typeOwner);
			}
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateShotB1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speedX = argv[2].as_real();
			double speedY = argv[3].as_real();
			int idShot = argv[4].as_int();
			int delay = argv[5].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);

			std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj.get()));
			pattern->SetSpeedX(speedX);
			pattern->SetSpeedY(speedY);
			obj->SetPattern(pattern);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateShotB2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speedX = argv[2].as_real();
			double speedY = argv[3].as_real();
			double accelX = argv[4].as_real();
			double accelY = argv[5].as_real();
			double maxSpeedX = argv[6].as_real();
			double maxSpeedY = argv[7].as_real();
			int idShot = argv[8].as_int();
			int delay = argv[9].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);

			std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj.get()));
			pattern->SetSpeedX(speedX);
			pattern->SetSpeedY(speedY);
			pattern->SetAccelerationX(accelX);
			pattern->SetAccelerationY(accelY);
			pattern->SetMaxSpeedX(maxSpeedX);
			pattern->SetMaxSpeedY(maxSpeedY);
			obj->SetPattern(pattern);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateShotOB1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		int tId = (int)argv[0].as_real();
		DxScriptRenderObject* tObj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(tId));
		if (tObj) {
			double posX = tObj->GetPosition().x;
			double posY = tObj->GetPosition().y;

			shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
			id = script->AddObject(obj);
			if (id != ID_INVALID) {
				stageController->GetShotManager()->AddShot(obj);
				double speedX = argv[1].as_real();
				double speedY = argv[2].as_real();
				int idShot = argv[3].as_int();
				int delay = argv[4].as_int();
				int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
					StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

				obj->SetX(posX);
				obj->SetY(posY);
				obj->SetShotDataID(idShot);
				obj->SetDelay(delay);
				obj->SetOwnerType(typeOwner);

				std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj.get()));
				pattern->SetSpeedX(speedX);
				pattern->SetSpeedY(speedY);
				obj->SetPattern(pattern);
			}
		}
	}
	return script->CreateRealValue(id);
}

gstd::value StgStageScript::Func_CreateLooseLaserA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgLooseLaserObject> obj(new StgLooseLaserObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speed = argv[2].as_real();
			double angle = argv[3].as_real();
			int length = argv[4].as_real();
			int width = argv[5].as_real();
			int idShot = argv[6].as_int();
			int delay = argv[7].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetSpeed(speed);
			obj->SetDirectionAngle(Math::DegreeToRadian(angle));
			obj->SetLength(length);
			obj->SetRenderWidth(width);
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);
		}
	}
	return script->CreateRealValue(id);
}

gstd::value StgStageScript::Func_CreateStraightLaserA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgStraightLaserObject> obj(new StgStraightLaserObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double angle = argv[2].as_real();
			int length = argv[3].as_int();
			int width = argv[4].as_int();
			int deleteFrame = argv[5].as_int();
			int idShot = argv[6].as_int();
			int delay = argv[7].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetLaserAngle(Math::DegreeToRadian(angle));
			obj->SetLength(length);
			obj->SetRenderWidth(width);
			obj->SetAutoDeleteFrame(deleteFrame);
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateCurveLaserA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		shared_ptr<StgCurveLaserObject> obj(new StgCurveLaserObject(stageController));
		id = script->AddObject(obj);
		if (id != ID_INVALID) {
			stageController->GetShotManager()->AddShot(obj);
			double posX = argv[0].as_real();
			double posY = argv[1].as_real();
			double speed = argv[2].as_real();
			double angle = argv[3].as_real();
			int length = argv[4].as_int();
			int width = argv[5].as_int();
			int idShot = argv[6].as_int();
			int delay = argv[7].as_int();
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

			obj->SetX(posX);
			obj->SetY(posY);
			obj->SetSpeed(speed);
			obj->SetDirectionAngle(Math::DegreeToRadian(angle));
			obj->SetLength(length);
			obj->SetRenderWidth(width);
			obj->SetShotDataID(idShot);
			obj->SetDelay(delay);
			obj->SetOwnerType(typeOwner);
		}
	}
	return script->CreateRealValue(id);
}

gstd::value StgStageScript::Func_SetShotIntersectionCircle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgIntersectionTarget::Type typeTarget = script->GetScriptType() == TYPE_PLAYER ?
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	float px = argv[0].as_real();
	float py = argv[1].as_real();
	float radius = argv[2].as_real();
	DxCircle circle(px, py, radius);

	//当たり判定
	shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
	if (target) {
		target->SetTargetType(typeTarget);
		target->SetCircle(circle);

		intersectionManager->AddTarget(target);
	}

	return value();
}
gstd::value StgStageScript::Func_SetShotIntersectionLine(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgIntersectionTarget::Type typeTarget = script->GetScriptType() == TYPE_PLAYER ?
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	float px1 = argv[0].as_real();
	float py1 = argv[1].as_real();
	float px2 = argv[2].as_real();
	float py2 = argv[3].as_real();
	float width = argv[4].as_real();
	DxWidthLine line(px1, py1, px2, py2, width);

	//当たり判定
	shared_ptr<StgIntersectionTarget_Line> target(new StgIntersectionTarget_Line);
	if (target) {
		target->SetTargetType(typeTarget);
		target->SetLine(line);

		intersectionManager->AddTarget(target);
	}

	return value();
}
gstd::value StgStageScript::Func_GetShotIdInCircleA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgShotManager* shotManager = stageController->GetShotManager();
	float px = argv[0].as_real();
	float py = argv[1].as_real();
	float radius = argv[2].as_real();
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ? StgShotObject::OWNER_ENEMY : StgShotObject::OWNER_PLAYER;

	std::vector<int> listID = shotManager->GetShotIdInCircle(typeOwner, px, py, radius);

	return script->CreateRealArrayValue(listID);
}
gstd::value StgStageScript::Func_GetShotIdInCircleA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgShotManager* shotManager = stageController->GetShotManager();
	float px = argv[0].as_real();
	float py = argv[1].as_real();
	float radius = argv[2].as_real();
	int target = argv[3].as_int();

	int typeOwner = StgShotObject::OWNER_NULL;
	switch (target) {
	case TARGET_ALL:typeOwner = StgShotObject::OWNER_NULL; break;
	case TARGET_PLAYER:typeOwner = StgShotObject::OWNER_PLAYER; break;
	case TARGET_ENEMY:typeOwner = StgShotObject::OWNER_ENEMY; break;
	}

	std::vector<int> listID = shotManager->GetShotIdInCircle(typeOwner, px, py, radius);

	return script->CreateRealArrayValue(listID);
}
gstd::value StgStageScript::Func_GetShotCount(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	int target = argv[0].as_int();
	int typeOwner = StgShotObject::OWNER_NULL;
	switch (target) {
	case TARGET_ALL:typeOwner = StgShotObject::OWNER_NULL; break;
	case TARGET_PLAYER:typeOwner = StgShotObject::OWNER_PLAYER; break;
	case TARGET_ENEMY:typeOwner = StgShotObject::OWNER_ENEMY; break;
	}

	size_t res = shotManager->GetShotCount(typeOwner);
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_SetShotAutoDeleteClip(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	RECT rect;
	rect.left = -(LONG)argv[0].as_int();
	rect.top = -(LONG)argv[1].as_int();
	rect.right = (LONG)argv[2].as_int();
	rect.bottom = (LONG)argv[3].as_int();
	infoStage->SetShotAutoDeleteClip(rect);

	return value();
}
gstd::value StgStageScript::Func_GetShotDataInfoA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	int idShot = argv[0].as_int();
	int target = argv[1].as_int();
	int type = argv[2].as_int();

	StgShotManager* shotManager = stageController->GetShotManager();
	StgShotDataList* dataList = (target == TARGET_PLAYER) ?
		shotManager->GetPlayerShotDataList() : shotManager->GetEnemyShotDataList();

	StgShotData* shotData = nullptr;
	if (dataList) shotData = dataList->GetData(idShot);

	gstd::value res;

	if (shotData == nullptr) {
		//script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));
		
		switch (type) {
		case INFO_EXISTS:
			res = script->CreateBooleanValue(false);
			break;
		case INFO_PATH:
			res = script->CreateStringValue(L"");
			break;
		case INFO_RECT:
		{
			LONG list[4] = { 0, 0, 0, 0 };
			res = script->CreateRealArrayValue(list, 4U);
			break;
		}
		case INFO_DELAY_COLOR:
		{
			byte list[4];
			ColorAccess::ToByteArray(0xffffffff, list);
			res = script->CreateRealArrayValue((byte*)(list + 1), 3U);
			break;
		}
		case INFO_BLEND:
			res = script->CreateRealValue(MODE_BLEND_NONE);
			break;
		case INFO_COLLISION:
			res = script->CreateRealValue(0);
			break;
		case INFO_COLLISION_LIST:
		{
			std::vector<gstd::value> listValue;
			int list[3] = { 0, 0, 0 };
			listValue.push_back(script->CreateRealArrayValue(list, 3U));
			res = script->CreateValueArrayValue(listValue);
			break;
		}
		case INFO_IS_FIXED_ANGLE:
			res = script->CreateBooleanValue(true);
			break;
		}
	}
	else {
		switch (type) {
		case INFO_EXISTS:
			res = script->CreateBooleanValue(true);
			break;
		case INFO_PATH:
			res = script->CreateStringValue(shotData->GetTexture()->GetName());
			break;
		case INFO_RECT:
		{
			RECT* rect = shotData->GetData(0)->GetSource();
			res = script->CreateRealArrayValue(reinterpret_cast<LONG*>(rect), 4U);
			break;
		}
		case INFO_DELAY_COLOR:
		{
			byte list[4];
			ColorAccess::ToByteArray(shotData->GetDelayColor(), list);
			res = script->CreateRealArrayValue((byte*)(list + 1), 3U);
			break;
		}
		case INFO_BLEND:
			res = script->CreateRealValue(shotData->GetRenderType());
			break;
		case INFO_COLLISION:
		{
			float radius = 0;
			DxCircle* listCircle = shotData->GetIntersectionCircleList();
			if (listCircle->GetR() > 0) {
				radius = listCircle->GetR();
			}
			res = script->CreateRealValue(radius);
			break;
		}
		case INFO_COLLISION_LIST:
		{
			DxCircle* listCircle = shotData->GetIntersectionCircleList();
			std::vector<gstd::value> listValue;
			float list[3] = { listCircle->GetR(), listCircle->GetX(), listCircle->GetY() };
			listValue.push_back(script->CreateRealArrayValue(list, 3U));
			res = script->CreateValueArrayValue(listValue);
			break;
		}
		case INFO_IS_FIXED_ANGLE:
			res = script->CreateBooleanValue(shotData->IsFixedAngle());
			break;
		}
	}

	return res;
}
gstd::value StgStageScript::Func_StartShotScript(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	auto scriptManager = stageController->GetScriptManager();

	if (scriptManager->GetShotScriptID() != StgControlScriptManager::ID_INVALID)
		script->RaiseError(L"A shot script was already started.");

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	int type = script->GetScriptType();
	shared_ptr<ManagedScript> idScript = scriptManager->LoadScript(path, StgStageScript::TYPE_SHOT);
	scriptManager->StartScript(idScript);
	scriptManager->SetShotScript(idScript);
	return value();
}

//STG共通関数：アイテム
gstd::value StgStageScript::Func_CreateItemA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	if (stageController->GetItemManager()->GetItemCount() >= StgItemManager::ITEM_MAX)
		return script->CreateRealValue(StgControlScript::ID_INVALID);

	int type = argv[0].as_int();
	shared_ptr<StgItemObject> obj = itemManager->CreateItem(type);

	int id = script->AddObject(obj);
	if (id != ID_INVALID) {
		itemManager->AddItem(obj);
		double posX = argv[1].as_real();
		double posY = argv[2].as_real();
		int64_t score = argv[3].as_int();
		D3DXVECTOR2 to = D3DXVECTOR2(posX, posY - 128);

		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateItemA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	if (stageController->GetItemManager()->GetItemCount() >= StgItemManager::ITEM_MAX)
		return script->CreateRealValue(StgControlScript::ID_INVALID);

	int type = argv[0].as_int();
	shared_ptr<StgItemObject> obj = itemManager->CreateItem(type);

	int id = script->AddObject(obj);
	if (id != ID_INVALID) {
		itemManager->AddItem(obj);
		double posX = argv[1].as_real();
		double posY = argv[2].as_real();
		D3DXVECTOR2 to = D3DXVECTOR2(argv[3].as_real(), argv[4].as_real());
		int64_t score = argv[5].as_int();

		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateItemU1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	if (stageController->GetItemManager()->GetItemCount() >= StgItemManager::ITEM_MAX)
		return script->CreateRealValue(StgControlScript::ID_INVALID);

	int type = StgItemObject::ITEM_USER;
	shared_ptr<StgItemObject_User> obj = std::dynamic_pointer_cast<StgItemObject_User>(itemManager->CreateItem(type));
	
	int id = script->AddObject(obj);
	if (id != ID_INVALID) {
		itemManager->AddItem(obj);
		int itemID = argv[0].as_int();
		double posX = argv[1].as_real();
		double posY = argv[2].as_real();
		int64_t score = argv[3].as_int();
		D3DXVECTOR2 to = D3DXVECTOR2(posX, posY - 128);

		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
		obj->SetImageID(itemID);
		obj->SetMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateItemU2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	if (stageController->GetItemManager()->GetItemCount() >= StgItemManager::ITEM_MAX)
		return script->CreateRealValue(StgControlScript::ID_INVALID);

	int type = StgItemObject::ITEM_USER;
	shared_ptr<StgItemObject_User> obj = std::dynamic_pointer_cast<StgItemObject_User>(itemManager->CreateItem(type));
	
	int id = script->AddObject(obj);
	if (id != ID_INVALID) {
		itemManager->AddItem(obj);
		int itemID = argv[0].as_int();
		double posX = argv[1].as_real();
		double posY = argv[2].as_real();
		D3DXVECTOR2 to = D3DXVECTOR2(argv[3].as_real(), argv[4].as_real());
		int64_t score = argv[5].as_int();

		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
		obj->SetImageID(itemID);
		obj->SetMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CreateItemScore(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	if (stageController->GetItemManager()->GetItemCount() >= StgItemManager::ITEM_MAX)
		return script->CreateRealValue(StgControlScript::ID_INVALID);

	int64_t score = argv[0].as_int();
	double posX = argv[1].as_real();
	double posY = argv[2].as_real();

	shared_ptr<StgItemObject_Score> obj(new StgItemObject_Score(stageController));
	int id = script->AddObject(obj);
	if (id != ID_INVALID) {
		itemManager->AddItem(obj);
		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetScore(score);
	}

	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_CollectAllItems(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();
	itemManager->CollectItemsAll();

	return value();
}
gstd::value StgStageScript::Func_CollectItemsByType(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	int type = argv[0].as_int();
	itemManager->CollectItemsByType(type);
	return value();
}
gstd::value StgStageScript::Func_CollectItemsInCircle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	float cx = argv[0].as_real();
	float cy = argv[1].as_real();
	float cr = argv[2].as_real();
	DxCircle circle(cx, cy, cr);
	itemManager->CollectItemsInCircle(circle);
	return value();
}
gstd::value StgStageScript::Func_CancelCollectItems(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	itemManager->CancelCollectItems();
	return value();
}
gstd::value StgStageScript::Func_StartItemScript(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	auto scriptManager = script->stageController_->GetScriptManager();

	if (scriptManager->GetItemScriptID() != StgControlScriptManager::ID_INVALID)
		script->RaiseError(L"An item script was already started.");

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	int type = script->GetScriptType();
	shared_ptr<ManagedScript> idScript = scriptManager->LoadScript(path, StgStageScript::TYPE_ITEM);
	scriptManager->StartScript(idScript);
	scriptManager->SetItemScript(idScript);
	return value();
}
gstd::value StgStageScript::Func_SetDefaultBonusItemEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	bool bEnable = argv[0].as_boolean();
	itemManager->SetDefaultBonusItemEnable(bEnable);
	return value();
}
gstd::value StgStageScript::Func_LoadItemData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = itemManager->LoadItemData(path);
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_ReloadItemData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = itemManager->LoadItemData(path, true);
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_SetItemIntersectionRadius(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgItemManager* itemManager = script->stageController_->GetItemManager();

	int r = argv[0].as_int();
	itemManager->SetItemIntersectionRadius(r);

	return value();
}

//STG共通関数：その他
gstd::value StgStageScript::Func_StartSlow(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int target = argv[0].as_int();
	int fps = argv[1].as_int();

	int slowTarget = PseudoSlowInformation::TARGET_ALL;
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
		PseudoSlowInformation::OWNER_PLAYER : PseudoSlowInformation::OWNER_ENEMY;

	ref_count_ptr<PseudoSlowInformation> infoSlow = stageController->GetSlowInformation();
	infoSlow->AddSlow(fps, typeOwner, slowTarget);

	return value();
}
gstd::value StgStageScript::Func_StopSlow(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int target = argv[0].as_int();

	int slowTarget = PseudoSlowInformation::TARGET_ALL;
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
		PseudoSlowInformation::OWNER_PLAYER : PseudoSlowInformation::OWNER_ENEMY;

	ref_count_ptr<PseudoSlowInformation> infoSlow = stageController->GetSlowInformation();
	infoSlow->RemoveSlow(typeOwner, slowTarget);

	return value();
}
template<bool PARTIAL>
gstd::value StgStageScript::Func_IsIntersected_Obj_Obj(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id1 = (int)argv[0].as_real();
	int id2 = (int)argv[1].as_real();

	StgIntersectionObject* obj1 = dynamic_cast<StgIntersectionObject*>(script->GetObjectPointer(id1));
	if (obj1 == nullptr) return script->CreateBooleanValue(false);

	StgIntersectionObject* obj2 = dynamic_cast<StgIntersectionObject*>(script->GetObjectPointer(id2));
	if (obj2 == nullptr) return script->CreateBooleanValue(false);

	std::vector<StgIntersectionTarget::ptr> listTarget1 = obj1->GetIntersectionTargetList();
	std::vector<StgIntersectionTarget::ptr> listTarget2 = obj2->GetIntersectionTargetList();

	bool res = false;
	for (auto& target1 : listTarget1) {
		for (auto& target2 : listTarget2) {
			res = StgIntersectionManager::IsIntersected(target1, target2);
			if (res && PARTIAL) goto chk_skip;
		}
	}

chk_skip:
	return script->CreateBooleanValue(res);
}


//STD共通関数：移動オブジェクト操作
gstd::value StgStageScript::Func_ObjMove_SetX(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double pos = argv[1].as_real();
		obj->SetPositionX(pos);

		DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(obj);
		if (objR)
			objR->SetX(pos);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double pos = argv[1].as_real();
		obj->SetPositionY(pos);

		DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(obj);
		if (objR)
			objR->SetY(pos);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetPosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double posX = argv[1].as_real();
		double posY = argv[2].as_real();
		obj->SetPositionX(posX);
		obj->SetPositionY(posY);

		DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(obj);
		if (objR) {
			objR->SetX(posX);
			objR->SetY(posY);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double speed = argv[1].as_real();

		StgMovePattern* pattern = obj->GetPattern().get();
		if (pattern) {
			if (auto patternXY = dynamic_cast<StgMovePattern_XY*>(pattern)) {
				double oldSpeed = patternXY->GetSpeed();
				if (oldSpeed != 0) {
					double newSpeedMul = speed / oldSpeed;
					patternXY->SetSpeedX(patternXY->GetSpeedX() * newSpeedMul);
					patternXY->SetSpeedY(patternXY->GetSpeedY() * newSpeedMul);
					goto set_exit;
				}
				else {
					obj->SetPattern(std::shared_ptr<StgMovePattern_Angle>(new StgMovePattern_Angle(obj)));
				}
			}
		}
		obj->SetSpeed(speed);
	}
set_exit:
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* objBase = script->GetObjectPointer(id);
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(objBase);
	if (obj) {
		double angle = Math::DegreeToRadian(argv[1].as_real());

		StgMovePattern* pattern = obj->GetPattern().get();
		if (pattern) {
			if (auto patternXY = dynamic_cast<StgMovePattern_XY*>(pattern)) {
				double speed = patternXY->GetSpeed();
				if (speed != 0) {
					patternXY->SetSpeedX(cos(angle) * speed);
					patternXY->SetSpeedY(sin(angle) * speed);
					goto set_exit;
				}
				else {
					obj->SetPattern(std::shared_ptr<StgMovePattern_Angle>(new StgMovePattern_Angle(obj)));
				}
			}
		}
		obj->SetDirectionAngle(angle);
	}
set_exit:
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAcceleration(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		std::shared_ptr<StgMovePattern_Angle> pattern = std::dynamic_pointer_cast<StgMovePattern_Angle>(obj->GetPattern());
		if (pattern == nullptr) {
			pattern = std::shared_ptr<StgMovePattern_Angle>(new StgMovePattern_Angle(obj));
			obj->SetPattern(pattern);
		}

		double param = argv[1].as_real();
		pattern->SetAcceleration(param);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAngularVelocity(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		std::shared_ptr<StgMovePattern_Angle> pattern = std::dynamic_pointer_cast<StgMovePattern_Angle>(obj->GetPattern());
		if (pattern == nullptr) {
			pattern = std::shared_ptr<StgMovePattern_Angle>(new StgMovePattern_Angle(obj));
			obj->SetPattern(pattern);
		}

		double param = argv[1].as_real();
		pattern->SetAngularVelocity(Math::DegreeToRadian(param));
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetMaxSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		std::shared_ptr<StgMovePattern_Angle> pattern = std::dynamic_pointer_cast<StgMovePattern_Angle>(obj->GetPattern());
		if (pattern == nullptr) {
			pattern = std::shared_ptr<StgMovePattern_Angle>(new StgMovePattern_Angle(obj));
			obj->SetPattern(pattern);
		}

		double param = argv[1].as_real();
		pattern->SetMaxSpeed(param);
	}
	return value();
}

gstd::value StgStageScript::Func_ObjMove_SetDestAtSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double tx = argv[1].as_real();
		double ty = argv[2].as_real();
		double speed = argv[3].as_real();

		std::shared_ptr<StgMovePattern_Line> pattern(new StgMovePattern_Line(obj));
		pattern->SetAtSpeed(tx, ty, speed);
		obj->SetPattern(pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetDestAtFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double tx = argv[1].as_real();
		double ty = argv[2].as_real();
		int frame = argv[3].as_int();

		std::shared_ptr<StgMovePattern_Line> pattern(new StgMovePattern_Line(obj));
		pattern->SetAtFrame(tx, ty, frame);
		obj->SetPattern(pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetDestAtWeight(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double tx = argv[1].as_real();
		double ty = argv[2].as_real();
		double weight = argv[3].as_real();
		double maxSpeed = argv[4].as_real();

		std::shared_ptr<StgMovePattern_Line> pattern(new StgMovePattern_Line(obj));
		pattern->SetAtWait(tx, ty, weight, maxSpeed);
		obj->SetPattern(pattern);
	}
	return value();
}

#define CMD_ADD(v, c) if (v != StgMovePattern::NO_CHANGE) pattern->AddCommand(std::make_pair(c, v));
#define CMD_ADD_ANG(v, c) if (v != StgMovePattern::NO_CHANGE) \
							pattern->AddCommand(std::make_pair(c, Math::DegreeToRadian(v)));
gstd::value StgStageScript::Func_ObjMove_AddPatternA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speed = argv[2].as_real();
		double angle = argv[3].as_real();

		std::shared_ptr<StgMovePattern_Angle> pattern(new StgMovePattern_Angle(obj));
		pattern->AddCommand(std::make_pair(StgMovePattern_Angle::SET_ZERO, 0));

		CMD_ADD(speed, StgMovePattern_Angle::SET_SPEED);
		CMD_ADD_ANG(angle, StgMovePattern_Angle::SET_ANGLE);

		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speed = argv[2].as_real();
		double angle = argv[3].as_real();
		double accele = argv[4].as_real();
		double angV = argv[5].as_real();
		double maxSpeed = argv[6].as_real();

		std::shared_ptr<StgMovePattern_Angle> pattern(new StgMovePattern_Angle(obj));

		CMD_ADD(speed, StgMovePattern_Angle::SET_SPEED);
		CMD_ADD_ANG(angle, StgMovePattern_Angle::SET_ANGLE);
		CMD_ADD(accele, StgMovePattern_Angle::SET_ACCEL);
		CMD_ADD_ANG(angV, StgMovePattern_Angle::SET_AGVEL);
		CMD_ADD(maxSpeed, StgMovePattern_Angle::SET_SPMAX);

		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA3(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speed = argv[2].as_real();
		double angle = argv[3].as_real();
		double accele = argv[4].as_real();
		double angV = argv[5].as_real();
		double maxSpeed = argv[6].as_real();
		int idShot = argv[7].as_int();

		std::shared_ptr<StgMovePattern_Angle> pattern(new StgMovePattern_Angle(obj));

		CMD_ADD(speed, StgMovePattern_Angle::SET_SPEED);
		CMD_ADD_ANG(angle, StgMovePattern_Angle::SET_ANGLE);
		CMD_ADD(accele, StgMovePattern_Angle::SET_ACCEL);
		CMD_ADD_ANG(angV, StgMovePattern_Angle::SET_AGVEL);
		CMD_ADD(maxSpeed, StgMovePattern_Angle::SET_SPMAX);

		pattern->SetShotDataID(idShot);
		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA4(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speed = argv[2].as_real();
		double angle = argv[3].as_real();
		double accele = argv[4].as_real();
		double angV = argv[5].as_real();
		double maxSpeed = argv[6].as_real();
		int idRelative = argv[7].as_int();
		int idShot = argv[8].as_int();

		std::shared_ptr<StgMovePattern_Angle> pattern(new StgMovePattern_Angle(obj));

		CMD_ADD(speed, StgMovePattern_Angle::SET_SPEED);
		CMD_ADD_ANG(angle, StgMovePattern_Angle::SET_ANGLE);
		CMD_ADD(accele, StgMovePattern_Angle::SET_ACCEL);
		CMD_ADD_ANG(angV, StgMovePattern_Angle::SET_AGVEL);
		CMD_ADD(maxSpeed, StgMovePattern_Angle::SET_SPMAX);

		pattern->SetShotDataID(idShot);
		if (idRelative != StgMovePattern::NO_CHANGE)
			pattern->SetRelativeObjectID(idRelative);
		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speedX = argv[2].as_real();
		double speedY = argv[3].as_real();

		std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj));
		pattern->AddCommand(std::make_pair(StgMovePattern_XY::SET_ZERO, 0));

		CMD_ADD(speedX, StgMovePattern_XY::SET_S_X);
		CMD_ADD(speedY, StgMovePattern_XY::SET_S_Y);

		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speedX = argv[2].as_real();
		double speedY = argv[3].as_real();
		double accelX = argv[4].as_real();
		double accelY = argv[5].as_real();
		double maxSpeedX = argv[6].as_real();
		double maxSpeedY = argv[7].as_real();

		std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj));

		CMD_ADD(speedX, StgMovePattern_XY::SET_S_X);
		CMD_ADD(speedY, StgMovePattern_XY::SET_S_Y);
		CMD_ADD(accelX, StgMovePattern_XY::SET_A_X);
		CMD_ADD(accelY, StgMovePattern_XY::SET_A_Y);
		CMD_ADD(maxSpeedX, StgMovePattern_XY::SET_M_X);
		CMD_ADD(maxSpeedY, StgMovePattern_XY::SET_M_Y);

		obj->AddPattern(frame, pattern);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB3(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		double speedX = argv[2].as_real();
		double speedY = argv[3].as_real();
		double accelX = argv[4].as_real();
		double accelY = argv[5].as_real();
		double maxSpeedX = argv[6].as_real();
		double maxSpeedY = argv[7].as_real();
		int idShot = argv[8].as_int();

		std::shared_ptr<StgMovePattern_XY> pattern(new StgMovePattern_XY(obj));

		CMD_ADD(speedX, StgMovePattern_XY::SET_S_X);
		CMD_ADD(speedY, StgMovePattern_XY::SET_S_Y);
		CMD_ADD(accelX, StgMovePattern_XY::SET_A_X);
		CMD_ADD(accelY, StgMovePattern_XY::SET_A_Y);
		CMD_ADD(maxSpeedX, StgMovePattern_XY::SET_M_X);
		CMD_ADD(maxSpeedY, StgMovePattern_XY::SET_M_Y);

		pattern->SetShotDataID(idShot);
		obj->AddPattern(frame, pattern);
	}
	return value();
}
#undef CMD_ADD
#undef CMD_ADD_ANG

gstd::value StgStageScript::Func_ObjMove_GetX(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	double pos = 0;
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj)
		pos = obj->GetPositionX();
	return script->CreateRealValue(pos);
}
gstd::value StgStageScript::Func_ObjMove_GetY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	double pos = 0;
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj)
		pos = obj->GetPositionY();
	return script->CreateRealValue(pos);
}
gstd::value StgStageScript::Func_ObjMove_GetSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	double speed = 0;
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj)
		speed = obj->GetSpeed();
	return script->CreateRealValue(speed);
}
gstd::value StgStageScript::Func_ObjMove_GetAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	double angle = 0;
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj)
		angle = Math::RadianToDegree(obj->GetDirectionAngle());
	return script->CreateRealValue(angle);
}
gstd::value StgStageScript::Func_ObjMove_SetSpeedX(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double param = argv[1].as_real();

		StgMovePattern* pattern = obj->GetPattern().get();
		if (pattern) {
			if (auto patternAng = dynamic_cast<StgMovePattern_Angle*>(pattern)) {
				double sx = param;
				double sy = pattern->GetSpeedY();
				patternAng->SetDirectionAngle(atan2(sy, sx));
				patternAng->SetSpeed(hypot(sx, sy));
			}
			else {
				obj->SetSpeedX(param);
			}
		}
		else {
			obj->SetSpeedX(param);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_GetSpeedX(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));

	double speed = 0.0;
	if (obj) {
		if (StgMovePattern* pattern = obj->GetPattern().get())
			speed = pattern->GetSpeedX();
	}

	return script->CreateRealValue(speed);
}
gstd::value StgStageScript::Func_ObjMove_SetSpeedY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double param = argv[1].as_real();

		StgMovePattern* pattern = obj->GetPattern().get();
		if (pattern) {
			if (auto patternAng = dynamic_cast<StgMovePattern_Angle*>(pattern)) {
				double sx = pattern->GetSpeedX();
				double sy = param;
				patternAng->SetDirectionAngle(atan2(sy, sx));
				patternAng->SetSpeed(hypot(sx, sy));
			}
			else {
				obj->SetSpeedY(param);
			}
		}
		else {
			obj->SetSpeedY(param);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_GetSpeedY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));

	double speed = 0.0;
	if (obj) {
		if (StgMovePattern* pattern = obj->GetPattern().get())
			speed = pattern->GetSpeedY();
	}

	return script->CreateRealValue(speed);
}
gstd::value StgStageScript::Func_ObjMove_SetSpeedXY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj) {
		double paramX = argv[1].as_real();
		double paramY = argv[2].as_real();

		StgMovePattern* pattern = obj->GetPattern().get();
		if (pattern) {
			if (auto patternAng = dynamic_cast<StgMovePattern_Angle*>(pattern)) {
				patternAng->SetDirectionAngle(atan2(paramY, paramX));
				patternAng->SetSpeed(hypot(paramX, paramY));
			}
			else {
				obj->SetSpeedX(paramX);
				obj->SetSpeedY(paramY);
			}
		}
		else {
			obj->SetSpeedX(paramX);
			obj->SetSpeedY(paramY);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetProcessMovement(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetEnableMovement(argv[1].as_boolean());
	return value();
}
gstd::value StgStageScript::Func_ObjMove_GetProcessMovement(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	bool res = obj ? obj->IsEnableMovement() : true;
	return script->CreateBooleanValue(res);
}

//STG共通関数：敵オブジェクト操作
gstd::value StgStageScript::Func_ObjEnemy_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	TypeObject type = (TypeObject)argv[0].as_int();
	shared_ptr<DxScriptObjectBase> obj;
	if (type == TypeObject::OBJ_ENEMY) {
		obj = shared_ptr<DxScriptObjectBase>(new StgEnemyObject(stageController));
	}
	else if (type == TypeObject::OBJ_ENEMY_BOSS) {
		shared_ptr<StgEnemyBossSceneObject> objScene = enemyManager->GetBossSceneObject();
		if (objScene == nullptr) {
			throw gstd::wexception(L"Cannot create a boss enemy as there is no active enemy boss scene object.");
		}

		shared_ptr<StgEnemyBossSceneData> data = objScene->GetActiveData();
		int id = data->GetEnemyBossIdInCreate();
		return script->CreateRealValue(id);
	}

	int id = ID_INVALID;
	if (obj) {
		id = script->AddObject(obj, false);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_ObjEnemy_Regist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	shared_ptr<StgEnemyObject> objEnemy = std::dynamic_pointer_cast<StgEnemyObject>(stageController->GetMainRenderObject(id));
	if (objEnemy) {
		StgEnemyManager* enemyManager = stageController->GetEnemyManager();
		enemyManager->AddEnemy(objEnemy);
		objEnemy->Activate();

		script->ActivateObject(objEnemy->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_GetInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = argv[1].as_int();

	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if (obj == nullptr) {
		switch (type) {
		case INFO_LIFE:
		case INFO_DAMAGE_RATE_SHOT:
		case INFO_DAMAGE_RATE_SPELL:
		case INFO_SHOT_HIT_COUNT:
			return script->CreateRealValue(0);
		}
		return value();
	}

	switch (type) {
	case INFO_LIFE:
		return script->CreateRealValue(obj->GetLife());
	case INFO_DAMAGE_RATE_SHOT:
		return script->CreateRealValue(obj->GetShotDamageRate());
	case INFO_DAMAGE_RATE_SPELL:
		return script->CreateRealValue(obj->GetSpellDamageRate());
	case INFO_SHOT_HIT_COUNT:
		return script->CreateRealValue(obj->GetIntersectedPlayerShotCount());
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetLife(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if (obj) {
		double param = argv[1].as_real();
		obj->SetLife(param);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_AddLife(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if (obj) {
		double inc = argv[1].as_real();
		obj->AddLife(inc);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetDamageRate(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if (obj) {
		double rateShot = argv[1].as_real();
		double rateSpell = argv[2].as_real();
		obj->SetDamageRate(rateShot, rateSpell);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_AddIntersectionCircleA(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float radius = argv[3].as_real();

		DxCircle circle(px, py, radius);

		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
			target->SetObject(obj);
			target->SetCircle(circle);
			obj->AddIntersectionRelativeTarget(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetIntersectionCircleToShot(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float radius = argv[3].as_real();

		DxCircle circle(px, py, radius);

		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
			target->SetObject(obj);
			target->SetCircle(circle);
			intersectionManager->AddEnemyTargetToShot(target);

			obj->AddReferenceToShotIntersection(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetIntersectionCircleToPlayer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float radius = argv[3].as_real();

		DxCircle circle(px, py, radius);

		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
			target->SetObject(obj);
			target->SetCircle(circle);
			intersectionManager->AddEnemyTargetToPlayer(target);

			obj->AddReferenceToPlayerIntersection(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_GetIntersectionCircleToShot(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	std::vector<gstd::value> listRes;
	float listValue[3];

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		auto listIntersection = obj->GetIntersectionListShot();

		for (auto itr = listIntersection->cbegin(); itr != listIntersection->cend(); ++itr) {
			if (auto ptr = itr->lock()) {
				DxCircle& circle = std::dynamic_pointer_cast<StgIntersectionTarget_Circle>(ptr)->GetCircle();
				listValue[0] = circle.GetX();
				listValue[1] = circle.GetY();
				listValue[2] = circle.GetR();
				listRes.push_back(script->CreateRealArrayValue(listValue, 3U));
			}
		}
	}

	return script->CreateValueArrayValue(listRes);
}
gstd::value StgStageScript::Func_ObjEnemy_GetIntersectionCircleToPlayer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	std::vector<gstd::value> listRes;
	float listValue[3];

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		auto listIntersection = obj->GetIntersectionListPlayer();

		for (auto itr = listIntersection->cbegin(); itr != listIntersection->cend(); ++itr) {
			if (auto ptr = itr->lock()) {
				DxCircle& circle = std::dynamic_pointer_cast<StgIntersectionTarget_Circle>(ptr)->GetCircle();
				listValue[0] = circle.GetX();
				listValue[1] = circle.GetY();
				listValue[2] = circle.GetR();
				listRes.push_back(script->CreateRealArrayValue(listValue, 3U));
			}
		}
	}

	return script->CreateValueArrayValue(listRes);
}
gstd::value StgStageScript::Func_ObjEnemy_SetEnableIntersectionPositionFetching(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	int id = (int)argv[0].as_real();
	shared_ptr<StgEnemyObject> obj = std::dynamic_pointer_cast<StgEnemyObject>(script->GetObject(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetEnableGetIntersectionPosition(bEnable);
	}

	return value();
}

//STG共通関数：敵ボスシーンオブジェクト操作
gstd::value StgStageScript::Func_ObjEnemyBossScene_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	shared_ptr<DxScriptObjectBase> obj(new StgEnemyBossSceneObject(stageController));

	int id = ID_INVALID;
	if (obj) {
		id = script->AddObject(obj, false);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_Regist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	shared_ptr<StgEnemyBossSceneObject> objScene = 
		std::dynamic_pointer_cast<StgEnemyBossSceneObject>(stageController->GetMainRenderObject(id));
	if (objScene) {
		enemyManager->SetBossSceneObject(objScene);
		objScene->Activate();
		script->ActivateObject(objScene->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_Add(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj) {
		int step = argv[1].as_int();
		std::wstring path = argv[2].as_string();
		path = PathProperty::GetUnique(path);

		shared_ptr<StgEnemyBossSceneData> data(new StgEnemyBossSceneData());
		data->SetPath(path);
		obj->AddData(step, data);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_LoadInThread(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->LoadAllScriptInThread();
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_GetInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = argv[1].as_int();

	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj == nullptr) {
		switch (type) {
		case INFO_IS_SPELL:
		case INFO_IS_LAST_SPELL:
		case INFO_IS_DURABLE_SPELL:
		case INFO_IS_LAST_STEP:
			return script->CreateBooleanValue(false);
		case INFO_TIMER:
		case INFO_TIMERF:
		case INFO_ORGTIMERF:
		case INFO_SPELL_SCORE:
		case INFO_REMAIN_STEP_COUNT:
		case INFO_ACTIVE_STEP_LIFE_COUNT:
		case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
		case INFO_ACTIVE_STEP_TOTAL_LIFE:
		case INFO_PLAYER_SHOOTDOWN_COUNT:
		case INFO_PLAYER_SPELL_COUNT:
		case INFO_CURRENT_LIFE:
		case INFO_CURRENT_LIFE_MAX:
			return script->CreateRealValue(0);
		case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
			return script->CreateRealArrayValue((int*)nullptr, 0);
		}
		return value();
	}

	shared_ptr<StgEnemyBossSceneData> sceneData = obj->GetActiveData();
	switch (type) {
	case INFO_IS_SPELL:
	{
		bool res = false;
		if (sceneData) res = sceneData->IsSpellCard();
		return script->CreateBooleanValue(res);
	}
	case INFO_IS_LAST_SPELL:
	{
		bool res = false;
		if (sceneData) res = sceneData->IsLastSpell();
		return script->CreateBooleanValue(res);
	}
	case INFO_IS_DURABLE_SPELL:
	{
		bool res = false;
		if (sceneData) res = sceneData->IsDurable();
		return script->CreateBooleanValue(res);
	}
	case INFO_TIMER:
	{
		double res = 0;
		if (sceneData) {
			int timer = sceneData->GetSpellTimer();
			res = timer < 0 ? 99 : (timer / STANDARD_FPS);
		}
		return script->CreateRealValue(res);
	}
	case INFO_TIMERF:
	{
		int res = 0;
		if (sceneData)
			res = sceneData->GetSpellTimer();
		return script->CreateRealValue(res);
	}
	case INFO_ORGTIMERF:
	{
		int res = 0;
		if (sceneData)
			res = sceneData->GetOriginalSpellTimer();
		return script->CreateRealValue(res);
	}
	case INFO_SPELL_SCORE:
	{
		int64_t res = 0;
		if (sceneData)
			res = sceneData->GetCurrentSpellScore();
		return script->CreateRealValue(res);
	}
	case INFO_REMAIN_STEP_COUNT:
		return script->CreateRealValue(obj->GetRemainStepCount());
	case INFO_ACTIVE_STEP_LIFE_COUNT:
		return script->CreateRealValue(obj->GetActiveStepLifeCount());
	case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
		return script->CreateRealValue(obj->GetActiveStepTotalMaxLife());
	case INFO_ACTIVE_STEP_TOTAL_LIFE:
		return script->CreateRealValue(obj->GetActiveStepTotalLife());
	case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
	{
		std::vector<double> listD = obj->GetActiveStepLifeRateList();
		return script->CreateRealArrayValue(listD);
	}
	case INFO_IS_LAST_STEP:
	{
		bool res = obj->GetRemainStepCount() == 0 && obj->GetActiveStepTotalLife() == 0;
		return script->CreateBooleanValue(res);
	}
	case INFO_PLAYER_SHOOTDOWN_COUNT:
	{
		int res = 0;
		if (sceneData)
			res = sceneData->GetPlayerShootDownCount();
		return script->CreateRealValue(res);
	}
	case INFO_PLAYER_SPELL_COUNT:
	{
		int res = 0;
		if (sceneData)
			res = sceneData->GetPlayerSpellCount();
		return script->CreateRealValue(res);
	}
	case INFO_CURRENT_LIFE:
	{
		double res = 0;
		if (sceneData) {
			int dataIndex = obj->GetDataIndex();
			res = obj->GetActiveStepLife(dataIndex);
		}
		return script->CreateRealValue(res);
	}
	case INFO_CURRENT_LIFE_MAX:
	{
		double res = 0;
		if (sceneData) {
			for (double iLife : sceneData->GetLifeList())
				res += iLife;
		}
		return script->CreateRealValue(res);
	}
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_SetSpellTimer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj) {
		shared_ptr<StgEnemyBossSceneData> sceneData = obj->GetActiveData();
		if (sceneData) {
			int timer = argv[1].as_int();
			sceneData->SetSpellTimer(timer);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_StartSpell(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj) {
		shared_ptr<StgEnemyBossSceneData> sceneData = obj->GetActiveData();
		if (sceneData) {
			sceneData->SetSpellCard(true);
			script->scriptManager_->RequestEventAll(EV_START_BOSS_SPELL);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_EndSpell(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if (obj) {
		shared_ptr<StgEnemyBossSceneData> sceneData = obj->GetActiveData();
		if (sceneData) {
			sceneData->SetSpellCard(false);
			script->scriptManager_->RequestEventAll(EV_END_BOSS_SPELL);
		}
	}
	return value();
}

//STG共通関数：弾オブジェクト操作
gstd::value StgStageScript::Func_ObjShot_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	int id = DxScript::ID_INVALID;
	if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
		TypeObject type = (TypeObject)argv[0].as_int();
		shared_ptr<StgShotObject> obj;
		if (type == TypeObject::OBJ_SHOT) {
			obj = shared_ptr<StgShotObject>(new StgNormalShotObject(stageController));
		}
		else if (type == TypeObject::OBJ_LOOSE_LASER) {
			obj = shared_ptr<StgShotObject>(new StgLooseLaserObject(stageController));
		}
		else if (type == TypeObject::OBJ_STRAIGHT_LASER) {
			obj = shared_ptr<StgShotObject>(new StgStraightLaserObject(stageController));
		}
		else if (type == TypeObject::OBJ_CURVE_LASER) {
			obj = shared_ptr<StgShotObject>(new StgCurveLaserObject(stageController));
		}

		id = ID_INVALID;
		if (obj) {
			int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
				StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;
			obj->SetOwnerType(typeOwner);
			id = script->AddObject(obj, false);
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_ObjShot_Regist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	shared_ptr<StgShotObject> objShot = std::dynamic_pointer_cast<StgShotObject>(stageController->GetMainRenderObject(id));
	if (objShot) {
		if (script->GetScriptType() == TYPE_PLAYER) {
			shared_ptr<StgPlayerObject> objPlayer = stageController->GetPlayerObject();
			if (objPlayer && !objPlayer->IsPermitShot()) return value();
		}

		StgShotManager* shotManager = stageController->GetShotManager();
		shotManager->AddShot(objShot);
		objShot->Activate();

		script->ActivateObject(objShot->GetObjectID(), true);
	}

	return value();
}

gstd::value StgStageScript::Func_ObjShot_SetOwnerType(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int typeOwner = argv[1].as_real();
		obj->SetOwnerType(typeOwner);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetAutoDelete(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bAutoDelete = argv[1].as_boolean();
		obj->SetAutoDelete(bAutoDelete);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_FadeDelete(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetFadeDelete();
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDeleteFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int frame = argv[1].as_int();
		obj->SetAutoDeleteFrame(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelay(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int delay = argv[1].as_int();
		obj->SetDelay(delay);
	}
	return value();
}

gstd::value StgStageScript::Func_ObjShot_SetSpellResist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bRegist = argv[1].as_boolean();
		double life = obj->GetLife();
		if (bRegist)
			life = StgShotObject::LIFE_SPELL_REGIST;
		else if (life >= StgShotObject::LIFE_SPELL_UNREGIST)
			life = StgShotObject::LIFE_SPELL_UNREGIST;
		obj->SetLife(life);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetGraphic(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int gr = argv[1].as_int();
		obj->SetShotDataID(gr);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetSourceBlendType(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int typeBlend = argv[1].as_int();
		obj->SetSourceBlendType((BlendMode)typeBlend);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDamage(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		double damage = argv[1].as_real();
		obj->SetDamage(damage);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetPenetration(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		double life = argv[1].as_real();
		obj->SetLife(life);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetEraseShot(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bErase = argv[1].as_boolean();
		obj->SetEraseShot(bErase);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetSpellFactor(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bErase = argv[1].as_boolean();
		obj->SetSpellFactor(bErase);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_ToItem(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) obj->ConvertToItem(false);
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionCircleA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgShotObject> obj = std::dynamic_pointer_cast<StgShotObject>(script->GetObject(id));

	if (obj && obj->GetDelay() <= 0) {
		obj->SetUserIntersectionMode(true);
		StgIntersectionTarget::Type typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ?
			StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

		StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

		float px = obj->GetPositionX();
		float py = obj->GetPositionY();
		float radius = argv[1].as_real();
		DxCircle circle(px, py, radius);

		//当たり判定
		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(typeTarget);
			target->SetCircle(circle);
			target->SetObject(obj);

			intersectionManager->AddTarget(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionCircleA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgShotObject> obj = std::dynamic_pointer_cast<StgShotObject>(script->GetObject(id));

	if (obj && obj->GetDelay() <= 0) {
		obj->SetUserIntersectionMode(true);
		StgIntersectionTarget::Type typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ?
			StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

		StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float radius = argv[3].as_real();
		DxCircle circle(px, py, radius);

		//当たり判定
		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(typeTarget);
			target->SetCircle(circle);
			target->SetObject(obj);

			intersectionManager->AddTarget(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionLine(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgShotObject> obj = std::dynamic_pointer_cast<StgShotObject>(script->GetObject(id));

	if (obj && obj->GetDelay() <= 0) {
		obj->SetUserIntersectionMode(true);
		StgIntersectionTarget::Type typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ?
			StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

		StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
		float px1 = argv[1].as_real();
		float py1 = argv[2].as_real();
		float px2 = argv[3].as_real();
		float py2 = argv[4].as_real();
		float width = argv[5].as_real();
		DxWidthLine line(px1, py1, px2, py2, width);

		//当たり判定
		shared_ptr<StgIntersectionTarget_Line> target(new StgIntersectionTarget_Line);
		if (target) {
			target->SetTargetType(typeTarget);
			target->SetObject(obj);
			target->SetLine(line);

			intersectionManager->AddTarget(target);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetIntersectionEnable(bEnable);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_GetIntersectionEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	bool res = false;
	if (obj)
		res = obj->IsIntersectionEnable();
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_ObjShot_SetItemChange(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetItemChangeEnable(bEnable);
	}
	return value();
}

gstd::value StgStageScript::Func_ObjShot_GetDelay(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	int res = 0;
	if (obj) 
		res = obj->GetDelay();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_ObjShot_GetDamage(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	double res = 0;
	if (obj)
		res = obj->GetDamage();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_ObjShot_GetPenetration(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	double res = 0;
	if (obj)
		res = obj->GetLife();
	return script->CreateRealValue(res);
}
gstd::value StgStageScript::Func_ObjShot_IsSpellResist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	bool res = false;
	if (obj)
		res = obj->GetLife() == StgShotObject::LIFE_SPELL_REGIST;
	return script->CreateBooleanValue(res);
}

gstd::value StgStageScript::Func_ObjShot_GetImageID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	int res = -1;
	if (obj) 
		res = obj->GetShotDataID();
	return script->CreateRealValue(res);
}

gstd::value StgStageScript::Func_ObjShot_SetIntersectionScaleX(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		float scale = argv[1].as_real();
		obj->SetHitboxScaleX(scale);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionScaleY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		float scale = argv[1].as_real();
		obj->SetHitboxScaleY(scale);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionScaleXY(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		float scaleX = argv[1].as_real();
		float scaleY = argv[2].as_real();
		obj->SetHitboxScaleX(scaleX);
		obj->SetHitboxScaleY(scaleY);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetPositionRounding(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) 
		obj->SetPositionRounding(argv[1].as_boolean());
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayMotionEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetEnableDelayMotion(argv[1].as_boolean());
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayGraphic(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		int gr = argv[1].as_int();
		obj->SetShotDataDelayID(gr);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayScaleParameter(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgShotObject::DelayParameter* delay = obj->GetDelayParameter();
		delay->scale = D3DXVECTOR3(argv[1].as_real(), argv[2].as_real(), argv[3].as_real());
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayAlphaParameter(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgShotObject::DelayParameter* delay = obj->GetDelayParameter();
		delay->alpha = D3DXVECTOR3(argv[1].as_real(), argv[2].as_real(), argv[3].as_real());
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayMode(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgShotObject::DelayParameter* delay = obj->GetDelayParameter();
		delay->type = argv[1].as_real();

		auto SetFunc = [](uint8_t typeLerp, StgShotObject::DelayParameter::lerp_func& target) {
			switch (typeLerp) {
			case Math::Lerp::SMOOTH:
				target = Math::Lerp::Smooth<float, float>;
				break;
			case Math::Lerp::SMOOTHER:
				target = Math::Lerp::Smoother<float, float>;
				break;
			case Math::Lerp::ACCELERATE:
				//Not a mistake
				target = Math::Lerp::Decelerate<float, float>;
				break;
			case Math::Lerp::DECELERATE:
				target = Math::Lerp::Accelerate<float, float>;
				break;
			case Math::Lerp::LINEAR:
			default:
				target = Math::Lerp::Linear<float, float>;
			}
		};
		
		SetFunc(argv[2].as_int(), delay->scaleLerpFunc);
		SetFunc(argv[3].as_int(), delay->alphaLerpFunc);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayColor(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgShotObject::DelayParameter* delay = obj->GetDelayParameter();
		delay->colorRep = argv[1].as_int();
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelayColoringEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgShotObject::DelayParameter* delay = obj->GetDelayParameter();
		delay->colorMix = argv[1].as_boolean();
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetGrazeInvalidFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	if (StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id))) {
		int frame = argv[1].as_int();
		obj->SetGrazeInvalidFrame(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetGrazeFrame(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	if (StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id))) {
		int frame = argv[1].as_int();
		obj->SetGrazeFrame(frame);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjShot_IsValidGraze(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();

	bool res = false;
	if (StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id)))
		res = obj->IsValidGraze();

	return script->CreateBooleanValue(res);
}


gstd::value StgStageScript::Func_ObjLaser_SetLength(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		int length = argv[1].as_int();
		obj->SetLength(length);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetRenderWidth(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		int width = argv[1].as_int();
		obj->SetRenderWidth(width);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetIntersectionWidth(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		int width = argv[1].as_int() / 2;
		obj->SetIntersectionWidth(width);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetInvalidLength(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		float start = argv[1].as_real();
		float end = argv[2].as_real();
		obj->SetInvalidLength(start, end);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetItemDistance(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		float dist = argv[1].as_real();
		obj->SetItemDistance(dist);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjLaser_GetLength(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	int length = 0;
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) 
		length = obj->GetLength();
	return script->CreateRealValue(length);
}
gstd::value StgStageScript::Func_ObjLaser_GetRenderWidth(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	int width = 0;
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) 
		width = obj->GetRenderWidth();
	return script->CreateRealValue(width);
}
gstd::value StgStageScript::Func_ObjLaser_GetIntersectionWidth(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	int width = 0;
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if (obj) 
		width = obj->GetIntersectionWidth();
	return script->CreateRealValue(width);
}
gstd::value StgStageScript::Func_ObjStLaser_SetAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		double angle = argv[1].as_real();
		obj->SetLaserAngle(Math::DegreeToRadian(angle));
	}
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_GetAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	double angle = 0;
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj)
		angle = Math::RadianToDegree(obj->GetLaserAngle());
	return script->CreateRealValue(angle);
}
gstd::value StgStageScript::Func_ObjStLaser_SetSource(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetSourceEnable(argv[1].as_boolean());
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_SetEnd(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetEndEnable(argv[1].as_boolean());
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_SetEndGraphic(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetEndGraphic(argv[1].as_int());
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_SetDelayScale(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		D3DXVECTOR2 scale = D3DXVECTOR2(argv[1].as_real(), argv[2].as_real());
		obj->SetSourceEndScale(scale);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_SetPermitExpand(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetLaserExpand(bEnable);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_GetPermitExpand(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	bool res = false;
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if (obj)
		res = obj->GetLaserExpand();
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_ObjCrLaser_SetTipDecrement(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		float dec = argv[1].as_real();
		//dec = std::clamp(dec, 0.0f, 1.0f);
		obj->SetTipDecrement(dec);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_GetNodePointer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	StgCurveLaserObject::LaserNode* res = nullptr;

	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		int index = argv[1].as_int();
		if (index >= 0) {
			std::list<StgCurveLaserObject::LaserNode>::iterator itr;
			bool bValidIndex = obj->GetNode(index, itr);
			res = bValidIndex ? &*itr : nullptr;
		}
	}

	return script->CreateIntValue((int64_t)res);
}
gstd::value StgStageScript::Func_ObjCrLaser_GetNodePointerList(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	std::vector<StgCurveLaserObject::LaserNode*> res;

	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		obj->GetNodePointerList(&res);
	}

	return script->CreateIntArrayValue(res);
}
gstd::value StgStageScript::Func_ObjCrLaser_GetNodePosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	float res[2] = { 0, 0 };
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			res[0] = ptr->pos.x;
			res[1] = ptr->pos.y;
		}
	}
	return script->CreateRealArrayValue(res, 2U);
}
gstd::value StgStageScript::Func_ObjCrLaser_GetNodeAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	double angle = 0.0;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			D3DXVECTOR2& vec = ptr->vertOff[0];
			angle = Math::RadianToDegree(atan2(vec.y, vec.x)) + 90.0;
		}
	}
	return script->CreateRealValue(angle);
}
gstd::value StgStageScript::Func_ObjCrLaser_GetNodeColor(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;

	D3DCOLOR color = 0xffffffff;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			color = ptr->color;
		}
	}

	byte colorArray[4];
	ColorAccess::ToByteArray(color, colorArray);
	return script->CreateRealArrayValue(colorArray, 4U);
}
gstd::value StgStageScript::Func_ObjCrLaser_SetNode(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			float x = argv[2].as_real();
			float y = argv[3].as_real();
			float angle = Math::DegreeToRadian(argv[4].as_real());
			D3DCOLOR color = argv[5].as_int();

			D3DXVECTOR2 rMove = D3DXVECTOR2(-sinf(angle), cosf(angle));

			StgCurveLaserObject::LaserNode node = obj->CreateNode(D3DXVECTOR2(x, y), rMove, color);
			*ptr = node;
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_SetNodePosition(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			float x = argv[2].as_real();
			float y = argv[3].as_real();
			ptr->pos = D3DXVECTOR2(x, y);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_SetNodeAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			float angle = Math::DegreeToRadian(argv[2].as_real());
			D3DXVECTOR2 rMove = D3DXVECTOR2(-sinf(angle), cosf(angle));

			StgCurveLaserObject::LaserNode node = obj->CreateNode(D3DXVECTOR2(0, 0), rMove);
			memcpy(ptr->vertOff, node.vertOff, sizeof(D3DXVECTOR2) * 2U);
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_SetNodeColor(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		StgCurveLaserObject::LaserNode* ptr = (StgCurveLaserObject::LaserNode*)argv[1].as_int();
		if (ptr) {
			D3DCOLOR color = argv[2].as_int();
			ptr->color = color;
		}
	}
	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_AddNode(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if (obj) {
		float x = argv[1].as_real();
		float y = argv[2].as_real();
		float angle = Math::DegreeToRadian(argv[3].as_real());
		D3DCOLOR color = argv[4].as_int();

		D3DXVECTOR2 rMove = D3DXVECTOR2(-sinf(angle), cosf(angle));

		StgCurveLaserObject::LaserNode node = obj->CreateNode(D3DXVECTOR2(x, y), rMove, color);
		obj->PushNode(node);
	}
	return value();
}

gstd::value StgStageScript::Func_ObjPatternShot_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeOwner = script->GetScriptType() == TYPE_PLAYER ?
		StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

	shared_ptr<StgPatternShotObjectGenerator> obj = std::make_shared<StgPatternShotObjectGenerator>();
	obj->SetTypeOwner(typeOwner);

	int id = script->AddObject(obj);
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_ObjPatternShot_Fire(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj)
		obj->FireSet(machine->data, stageController, nullptr);
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_FireReturn(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));

	std::vector<int> res;
	if (obj) 
		obj->FireSet(machine->data, stageController, &res);

	return script->CreateRealArrayValue(res);
}
gstd::value StgStageScript::Func_ObjPatternShot_SetParentObject(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		int idParent = argv[1].as_int();
		shared_ptr<StgMoveObject> objParent = std::dynamic_pointer_cast<StgMoveObject>(script->GetObject(idParent));
		if (objParent) obj->SetParent(objParent);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetPatternType(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetTypePattern(argv[1].as_int());
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetShotType(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		TypeObject type = (TypeObject)argv[1].as_int();
		obj->SetTypeShot(type);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetInitialBlendMode(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetBlendType((BlendMode)argv[1].as_int());
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetShotCount(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		int way = argv[1].as_int();
		int stack = argv[2].as_int();
		obj->SetWayStack((size_t)std::max(0, way), (size_t)std::max(0, stack));
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float base = argv[1].as_real();
		float arg = argv[2].as_real();
		obj->SetSpeed(base, arg);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetAngle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float base = Math::DegreeToRadian(argv[1].as_real());
		float arg = Math::DegreeToRadian(argv[2].as_real());
		obj->SetAngle(base, arg);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetBasePoint(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float x = argv[1].as_real();
		float y = argv[2].as_real();
		obj->SetBasePoint(x, y);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetBasePointOffset(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float x = argv[1].as_real();
		float y = argv[2].as_real();
		obj->SetOffsetFromBasePoint(x, y);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetBasePointOffsetCircle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float angle = Math::DegreeToRadian(argv[1].as_real());
		float radius = argv[2].as_real();
		obj->SetOffsetFromBasePoint(radius * cos(angle), radius * sin(angle));
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetShootRadius(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		float r = argv[1].as_real();
		obj->SetRadiusFromFirePoint(r);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetDelay(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		int delay = argv[1].as_int();
		obj->SetDelay(delay);
	}
	return value();
}
/*
gstd::value StgStageScript::Func_ObjPatternShot_SetDelayMotion(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj)
		obj->SetDelayMotion(argv[1].as_boolean());
	return value();
}
*/
gstd::value StgStageScript::Func_ObjPatternShot_SetGraphic(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		int graphic = argv[1].as_int();
		obj->SetGraphic(graphic);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetLaserParameter(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(id));
	if (obj) {
		int width = argv[1].as_int();
		int length = argv[2].as_int();
		obj->SetLaserArgument(width, length);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_CopySettings(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int idDst = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* objDst = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(idDst));
	if (objDst) {
		int idSrc = (int)argv[1].as_real();
		StgPatternShotObjectGenerator* objSrc = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(idSrc));
		if (objSrc)
			objDst->CopyFrom(objSrc);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_AddTransform(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int idDst = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(idDst));
	if (obj) {
		int typeAct = argv[1].as_int();

		StgPatternShotTransform transform;
		transform.act = (uint8_t)typeAct;
		transform.param_s[0] = argv[2].as_int();
		transform.param_s[1] = argv[3].as_int();
		transform.param_s[2] = argv[4].as_int();
		transform.param_d[0] = argv[5].as_real();
		transform.param_d[1] = argv[6].as_real();
		transform.param_d[2] = argv[7].as_real();
		obj->AddTransformation(transform);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPatternShot_SetTransform(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int idDst = (int)argv[0].as_real();
	StgPatternShotObjectGenerator* obj = dynamic_cast<StgPatternShotObjectGenerator*>(script->GetObjectPointer(idDst));
	if (obj) {
		int slot = argv[1].as_int();
		int typeAct = argv[2].as_int();

		StgPatternShotTransform transform;
		transform.act = (uint8_t)typeAct;
		transform.param_s[0] = argv[3].as_int();
		transform.param_s[1] = argv[4].as_int();
		transform.param_s[2] = argv[5].as_int();
		transform.param_d[0] = argv[6].as_real();
		transform.param_d[1] = argv[7].as_real();
		transform.param_d[2] = argv[8].as_real();
		obj->SetTransformation(slot, transform);
	}
	return value();
}

//STG共通関数：アイテムオブジェクト操作
gstd::value StgStageScript::Func_ObjItem_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	int type = argv[0].as_int();
	shared_ptr<StgItemObject> obj;
	if (type == StgItemObject::ITEM_USER) {
		obj = shared_ptr<StgItemObject>(new StgItemObject_User(stageController));
	}

	int id = ID_INVALID;
	if (obj) {
		id = script->AddObject(obj, false);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStageScript::Func_ObjItem_Regist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	shared_ptr<StgItemObject> objItem = std::dynamic_pointer_cast<StgItemObject>(stageController->GetMainRenderObject(id));
	if (objItem) {
		StgItemManager* itemManager = stageController->GetItemManager();
		itemManager->AddItem(objItem);
		objItem->Activate();

		script->ActivateObject(objItem->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetItemID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject_User* obj = dynamic_cast<StgItemObject_User*>(script->GetObjectPointer(id));
	if (obj) {
		int id = argv[1].as_int();
		obj->SetImageID(id);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetRenderScoreEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetChangeItemScore(bEnable);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetAutoCollectEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if (obj) {
		bool bEnable = argv[1].as_boolean();
		obj->SetPermitMoveToPlayer(bEnable);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetDefinedMovePatternA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if (obj) {
		int type = argv[1].as_int();
		std::shared_ptr<StgMovePattern_Item> move(new StgMovePattern_Item(obj));
		move->SetItemMoveType(type);
		obj->SetPattern(move);
	}
	return value();
}

gstd::value StgStageScript::Func_ObjItem_GetInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = argv[1].as_int();

	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if (obj == nullptr) {
		switch (type) {
		case INFO_ITEM_SCORE:
			return script->CreateRealValue(0);
		case INFO_ITEM_MOVE_TYPE:
			return script->CreateRealValue(StgMovePattern_Item::MOVE_NONE);
		}
		return value();
	}

	switch (type) {
	case INFO_ITEM_SCORE:
		return script->CreateRealValue(obj->GetScore());
	case INFO_ITEM_MOVE_TYPE:
		return script->CreateRealValue(obj->GetMoveType());
	}

	return value();
}

//STG共通関数：自機オブジェクト操作
gstd::value StgStageScript::Func_ObjPlayer_AddIntersectionCircleA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerObject> obj = std::dynamic_pointer_cast<StgPlayerObject>(script->GetObject(id));
	if (obj) {
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float rHit = argv[3].as_real();
		float rGraze = argv[4].as_real();

		DxCircle circle(px, py, rHit);

		shared_ptr<StgIntersectionTarget_Player> targetHit(new StgIntersectionTarget_Player(false));
		targetHit->SetObject(obj);
		targetHit->SetCircle(circle);
		obj->AddIntersectionRelativeTarget(targetHit);

		circle.SetR(rHit + rGraze);
		shared_ptr<StgIntersectionTarget_Player> targetGraze(new StgIntersectionTarget_Player(true));
		targetGraze->SetObject(obj);
		targetGraze->SetCircle(circle);
		obj->AddIntersectionRelativeTarget(targetGraze);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPlayer_AddIntersectionCircleA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerObject> obj = std::dynamic_pointer_cast<StgPlayerObject>(script->GetObject(id));
	if (obj) {
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float rGraze = argv[3].as_real();

		DxCircle circle(px, py, 0);

		circle.SetR(rGraze);
		shared_ptr<StgIntersectionTarget_Player> targetGraze(new StgIntersectionTarget_Player(true));
		targetGraze->SetObject(obj);
		targetGraze->SetCircle(circle);
		obj->AddIntersectionRelativeTarget(targetGraze);
	}
	return value();
}
gstd::value StgStageScript::Func_ObjPlayer_ClearIntersection(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerObject> obj = std::dynamic_pointer_cast<StgPlayerObject>(script->GetObject(id));
	if (obj)
		obj->ClearIntersectionRelativeTarget();
	return value();
}

//STG共通関数：当たり判定オブジェクト操作
gstd::value StgStageScript::Func_ObjCol_IsIntersected(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	bool res = false;
	StgIntersectionObject* obj = dynamic_cast<StgIntersectionObject*>(script->GetObjectPointer(id));
	if (obj)
		res = obj->IsIntersected();
	return script->CreateBooleanValue(res);
}
gstd::value StgStageScript::Func_ObjCol_GetListOfIntersectedEnemyID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::vector<int> listObjectID;
	StgIntersectionObject* obj = dynamic_cast<StgIntersectionObject*>(script->GetObjectPointer(id));
	if (obj) {
		std::vector<weak_ptr<StgIntersectionObject>>& listIntersection = obj->GetIntersectedIdList();
		for (auto& wPtr : listIntersection) {
			if (auto ptr = wPtr.lock()) {
				if (StgEnemyObject* objEnemy = dynamic_cast<StgEnemyObject*>(ptr.get()))
					listObjectID.push_back(objEnemy->GetDxScriptObjectID());
			}
		}
	}
	return script->CreateRealArrayValue(listObjectID);
}
gstd::value StgStageScript::Func_ObjCol_GetListOfIntersectedShotID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = argv[1].as_int();
	std::vector<int> listObjectID;
	StgIntersectionObject* obj = dynamic_cast<StgIntersectionObject*>(script->GetObjectPointer(id));
	if (obj) {
		std::vector<weak_ptr<StgIntersectionObject>>& listIntersection = obj->GetIntersectedIdList();
		for (auto& wPtr : listIntersection) {
			if (auto ptr = wPtr.lock()) {
				if (StgShotObject* objShot = dynamic_cast<StgShotObject*>(ptr.get())) {
					if (objShot->GetOwnerType() == type)
						listObjectID.push_back(objShot->GetDxScriptObjectID());
				}
			}
		}
	}
	return script->CreateRealArrayValue(listObjectID);
}
gstd::value StgStageScript::Func_ObjCol_GetIntersectedCount(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	size_t res = 0;
	shared_ptr<StgIntersectionObject> obj = std::dynamic_pointer_cast<StgIntersectionObject>(script->GetObject(id));
	if (obj) {
		for (auto& wPtr : obj->GetIntersectedIdList()) {
			if (!wPtr.expired()) ++res;
		}
	}
	return script->CreateRealValue(res);
}

/**********************************************************
//StgSystemScript
**********************************************************/
function const stgSystemFunction[] =
{
	//関数：


	//定数
	{ "__stgSystemFunction__",constant<0>::func, 0 },
};
StgStageSystemScript::StgStageSystemScript(StgStageController* stageController) : StgStageScript(stageController) {
	typeScript_ = TYPE_SYSTEM;
	_AddFunction(stgSystemFunction, sizeof(stgSystemFunction) / sizeof(function));
}
StgStageSystemScript::~StgStageSystemScript() {}

/**********************************************************
//StgStageItemScript
**********************************************************/
function const stgItemFunction[] =
{
	//定数
	{ "EV_GET_ITEM", constant<StgStageItemScript::EV_GET_ITEM>::func, 0 },
	{ "EV_DELETE_SHOT_IMMEDIATE", constant<StgStageItemScript::EV_DELETE_SHOT_IMMEDIATE>::func, 0 },
	{ "EV_DELETE_SHOT_TO_ITEM", constant<StgStageItemScript::EV_DELETE_SHOT_TO_ITEM>::func, 0 },
	{ "EV_DELETE_SHOT_FADE", constant<StgStageItemScript::EV_DELETE_SHOT_FADE>::func, 0 },

	{ "EV_GRAZE", constant<StgStagePlayerScript::EV_GRAZE>::func, 0 },
};
StgStageItemScript::StgStageItemScript(StgStageController* stageController) : StgStageScript(stageController) {
	typeScript_ = TYPE_ITEM;
	_AddFunction(stgItemFunction, sizeof(stgItemFunction) / sizeof(function));
}
StgStageItemScript::~StgStageItemScript() {}

/**********************************************************
//StgStageShotScript
**********************************************************/
function const stgShotFunction[] =
{
	//定数
	{ "EV_DELETE_SHOT_IMMEDIATE", constant<StgStageScript::EV_DELETE_SHOT_IMMEDIATE>::func, 0 },
	{ "EV_DELETE_SHOT_TO_ITEM", constant<StgStageScript::EV_DELETE_SHOT_TO_ITEM>::func, 0 },
	{ "EV_DELETE_SHOT_FADE", constant<StgStageScript::EV_DELETE_SHOT_FADE>::func, 0 },
	
	{ "SetShotDeleteEventEnable", StgStageShotScript::Func_SetShotDeleteEventEnable, 2 },
};
StgStageShotScript::StgStageShotScript(StgStageController* stageController) : StgStageScript(stageController) {
	typeScript_ = TYPE_SHOT;
	_AddFunction(stgShotFunction, sizeof(stgShotFunction) / sizeof(function));
}
StgStageShotScript::~StgStageShotScript() {}

gstd::value StgStageShotScript::Func_SetShotDeleteEventEnable(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStageShotScript* script = (StgStageShotScript*)machine->data;
	int type = argv[0].as_int();
	bool bEnable = argv[1].as_boolean();

	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();
	shotManager->SetDeleteEventEnableByType(type, bEnable);

	return value();
}

/**********************************************************
//StgPlayerScript
**********************************************************/
function const stgPlayerFunction[] =
{
	//関数：
	{ "CreatePlayerShotA1", StgStagePlayerScript::Func_CreatePlayerShotA1, 7 },
	{ "CallSpell", StgStagePlayerScript::Func_CallSpell, 0 },
	{ "LoadPlayerShotData", StgStagePlayerScript::Func_LoadPlayerShotData, 1 },
	{ "ReloadPlayerShotData", StgStagePlayerScript::Func_ReloadPlayerShotData, 1 },
	{ "GetSpellManageObject", StgStagePlayerScript::Func_GetSpellManageObject, 0 },

	{ "KillPlayer", StgStagePlayerScript::Func_KillPlayer, 0 },

	//自機専用関数：スペルオブジェクト操作
	{ "ObjSpell_Create", StgStagePlayerScript::Func_ObjSpell_Create, 0 },
	{ "ObjSpell_Regist", StgStagePlayerScript::Func_ObjSpell_Regist, 1 },
	{ "ObjSpell_SetDamage", StgStagePlayerScript::Func_ObjSpell_SetDamage, 2 },
	{ "ObjSpell_SetPenetration", StgStagePlayerScript::Func_ObjSpell_SetPenetration, 2 },
	{ "ObjSpell_SetEraseShot", StgStagePlayerScript::Func_ObjSpell_SetEraseShot, 2 },
	{ "ObjSpell_SetIntersectionCircle", StgStagePlayerScript::Func_ObjSpell_SetIntersectionCircle, 4 },
	{ "ObjSpell_SetIntersectionLine", StgStagePlayerScript::Func_ObjSpell_SetIntersectionLine, 6 },

	//定数
	{ "EV_REQUEST_SPELL", constant<StgStagePlayerScript::EV_REQUEST_SPELL>::func, 0 },
	{ "EV_GRAZE", constant<StgStagePlayerScript::EV_GRAZE>::func, 0 },
	{ "EV_HIT", constant<StgStagePlayerScript::EV_HIT>::func, 0 },
	{ "EV_DELETE_SHOT_PLAYER", constant<StgStagePlayerScript::EV_DELETE_SHOT_PLAYER>::func, 0 },

	{ "EV_GET_ITEM", constant<StgStageItemScript::EV_GET_ITEM>::func, 0 },
};
StgStagePlayerScript::StgStagePlayerScript(StgStageController* stageController) : StgStageScript(stageController) {
	typeScript_ = TYPE_PLAYER;
	_AddFunction(stgPlayerFunction, sizeof(stgPlayerFunction) / sizeof(function));
}
StgStagePlayerScript::~StgStagePlayerScript() {}

//自機専用関数
gstd::value StgStagePlayerScript::Func_CreatePlayerShotA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = StgControlScript::ID_INVALID;

	shared_ptr<StgPlayerObject> objPlayer = stageController->GetPlayerObject();
	if (objPlayer) {
		if (stageController->GetShotManager()->GetShotCountAll() < StgShotManager::SHOT_MAX) {
			shared_ptr<StgNormalShotObject> obj(new StgNormalShotObject(stageController));
			id = script->AddObject(obj);
			if (id != ID_INVALID) {
				stageController->GetShotManager()->AddShot(obj);
				double posX = argv[0].as_real();
				double posY = argv[1].as_real();
				double speed = argv[2].as_real();
				double angle = argv[3].as_real();
				double damage = argv[4].as_real();
				double life = argv[5].as_real();
				int idShot = argv[6].as_int();

				obj->SetOwnerType(StgShotObject::OWNER_PLAYER);
				obj->SetX(posX);
				obj->SetY(posY);
				obj->SetSpeed(speed);
				obj->SetDirectionAngle(Math::DegreeToRadian(angle));
				obj->SetLife(life);
				obj->SetDamage(damage);
				obj->SetShotDataID(idShot);
			}
		}
	}
	return script->CreateRealValue(id);
}
gstd::value StgStagePlayerScript::Func_CallSpell(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	shared_ptr<StgPlayerObject> objPlayer = script->stageController_->GetPlayerObject();
	if (objPlayer)
		objPlayer->CallSpell();
	return value();
}
gstd::value StgStagePlayerScript::Func_LoadPlayerShotData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadPlayerShotData(path);
	return script->CreateBooleanValue(res);
}
gstd::value StgStagePlayerScript::Func_ReloadPlayerShotData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadPlayerShotData(path, true);
	return script->CreateBooleanValue(res);
}
gstd::value StgStagePlayerScript::Func_GetSpellManageObject(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	shared_ptr<StgPlayerObject> obj = stageController->GetPlayerObject();
	int id = ID_INVALID;
	if (obj) {
		shared_ptr<StgPlayerSpellManageObject> objManage = obj->GetSpellManageObject();
		if (objManage)
			id = objManage->GetObjectID();
	}
	return script->CreateRealValue(id);
}

gstd::value StgStagePlayerScript::Func_KillPlayer(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	shared_ptr<StgPlayerObject> objPlayer = script->stageController_->GetPlayerObject();
	if (objPlayer) {
		if (objPlayer->GetState() == StgPlayerObject::STATE_NORMAL)
			objPlayer->KillSelf(true);
	}
	return value();
}

//自機専用関数：スペルオブジェクト操作
gstd::value StgStagePlayerScript::Func_ObjSpell_Create(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	shared_ptr<StgPlayerSpellObject> obj(new StgPlayerSpellObject(stageController));

	int id = ID_INVALID;
	if (obj) {
		id = script->AddObject(obj, false);
	}
	return script->CreateRealValue(id);
}
gstd::value StgStagePlayerScript::Func_ObjSpell_Regist(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		script->ActivateObject(objSpell->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetDamage(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		double damage = argv[1].as_real();
		objSpell->SetDamage(damage);
	}
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetPenetration(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		double life = argv[1].as_real();
		objSpell->SetLife(life);
	}
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetEraseShot(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		bool bEraseShot = argv[1].as_boolean();
		objSpell->SetEraseShot(bEraseShot);
	}
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetIntersectionCircle(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
		float px = argv[1].as_real();
		float py = argv[2].as_real();
		float radius = argv[3].as_real();
		DxCircle circle(px, py, radius);

		shared_ptr<StgIntersectionTarget_Circle> target(new StgIntersectionTarget_Circle);
		if (target) {
			target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SPELL);
			target->SetObject(objSpell);
			target->SetCircle(circle);

			intersectionManager->AddTarget(target);
		}
	}
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetIntersectionLine(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	shared_ptr<StgPlayerSpellObject> objSpell =
		std::dynamic_pointer_cast<StgPlayerSpellObject>(stageController->GetMainRenderObject(id));
	if (objSpell) {
		StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
		float px1 = argv[1].as_real();
		float py1 = argv[2].as_real();
		float px2 = argv[3].as_real();
		float py2 = argv[4].as_real();
		float width = argv[5].as_real();
		DxWidthLine line(px1, py1, px2, py2, width);

		shared_ptr<StgIntersectionTarget_Line> target(new StgIntersectionTarget_Line);
		if (target) {
			target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SPELL);
			target->SetObject(objSpell);
			target->SetLine(line);

			intersectionManager->AddTarget(target);
		}
	}
	return value();
}