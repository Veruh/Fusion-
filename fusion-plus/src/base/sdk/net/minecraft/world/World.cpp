#include "World.h"

#include "sdk/sdk.h"
#include "java/java.h"
#include "sdk/java/util/List.h"
#include "sdk/net/minecraft/util/Vec3.h"
#include "sdk/net/minecraft/util/MovingObjectPosition.h"

#include "util/logger.h"

CWorld::CWorld() 
{
	this->Class = StrayCache::world_class;
	this->FieldIDs["playerEntities"] = StrayCache::world_playerEntities;
	this->MethodIDs["rayTraceBlocks"] = StrayCache::world_rayTraceBlocks;
	this->MethodIDs["getBlockState"] = StrayCache::world_getBlockState;
}

jclass CWorld::GetClass()
{
	return this->Class;
}

jobject CWorld::GetInstance()
{
	return Java::Env->GetObjectField(SDK::Minecraft->GetInstance(), SDK::Minecraft->FieldIDs["theWorld"]);
}

std::vector<CEntityPlayer> CWorld::GetPlayerList()
{
	std::vector<CEntityPlayer> finalList;
	jobject world = this->GetInstance();
	if (!world || Java::Env->ExceptionCheck()) return finalList;
	jobject playerEntitiesList = Java::Env->GetObjectField(world, this->FieldIDs["playerEntities"]);
	if (!playerEntitiesList || Java::Env->ExceptionCheck()) return finalList;
	List playerList(playerEntitiesList);
	if (Java::Env->ExceptionCheck()) return finalList;
	jobjectArray playerEntities = playerList.toArray();
	if (!playerEntities || Java::Env->ExceptionCheck()) return finalList;
	int size = Java::Env->GetArrayLength(playerEntities);
	if (Java::Env->ExceptionCheck()) return finalList;
	
	for (int i = 0; i < size; i++)
	{
		jobject obj_player = Java::Env->GetObjectArrayElement(playerEntities, i);
		if (Java::Env->ExceptionCheck()) return {};
		if (!obj_player) continue;

		CEntityPlayer player =  CEntityPlayer::CEntityPlayer(obj_player);
		finalList.push_back(player);
	}

	Java::Env->DeleteLocalRef(playerEntitiesList);
	Java::Env->DeleteLocalRef(playerEntities);

	return finalList;
}

bool CWorld::rayTraceBlocks(Vector3 from, Vector3 to, Vector3& result, bool stopOnLiquid, bool ignoreBlockWithoutBoundingBox, bool returnLastUncollidableBlock)
{
	// SPECIAL CASE
	jclass cls;
	Java::AssignClass(StrayCache::vec3_class_name, cls);

	jmethodID init = Java::Env->GetMethodID(cls, "<init>", "(DDD)V");
	jobject j_to = Java::Env->NewObject(cls, init, (jdouble)(double)to.x, (jdouble)(double)to.y, (jdouble)(double)to.z);
	jobject j_from = Java::Env->NewObject(cls, init, (jdouble)(double)from.x, (jdouble)(double)from.y, (jdouble)(double)from.z);

	jobject movingObjPos_j = Java::Env->CallObjectMethod(
		this->GetInstance(),
		this->MethodIDs["rayTraceBlocks"],
		j_from,
		j_to,
		stopOnLiquid,
		ignoreBlockWithoutBoundingBox,
		returnLastUncollidableBlock
	);

	if (movingObjPos_j == NULL) {
		Java::Env->DeleteLocalRef(j_to);
		Java::Env->DeleteLocalRef(j_from);
		Java::Env->DeleteLocalRef(cls);
		return false;
	}

	CMovingObjectPosition movingObjPos = CMovingObjectPosition(movingObjPos_j);
	CVec3 a = movingObjPos.GetBlockPosition();
	result = a.GetNativeVector3();
	Java::Env->DeleteLocalRef(j_to);
	Java::Env->DeleteLocalRef(j_from);
	Java::Env->DeleteLocalRef(cls);
	return true;
}

CIBlockState CWorld::GetBlockState(CBlockPos pos)
{
	return CIBlockState(Java::Env->CallObjectMethod(this->GetInstance(), this->MethodIDs["getBlockState"], pos.GetInstance()));
}

