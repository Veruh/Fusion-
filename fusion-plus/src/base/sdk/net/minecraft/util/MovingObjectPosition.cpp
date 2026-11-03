#include "MovingObjectPosition.h"

#include "java/java.h"
#include "sdk/strayCache.h"
#include <iostream>

CMovingObjectPosition::CMovingObjectPosition()
{
	if (!StrayCache::initialized) StrayCache::Initialize();
	this->Class = StrayCache::movingObjectPosition_class;
}

CMovingObjectPosition::CMovingObjectPosition(jobject instance) : CMovingObjectPosition()
{
	this->Instance = instance;
}

jclass CMovingObjectPosition::GetClass()
{
	return this->Class;
}

jobject CMovingObjectPosition::GetInstance()
{
	return this->Instance;
}

CVec3 CMovingObjectPosition::GetBlockPosition()
{
	return CVec3(Java::Env->GetObjectField(this->GetInstance(), StrayCache::movingObjectPosition_hitVec));
}

bool CMovingObjectPosition::IsTypeOfBlock()
{
	if (!this->GetInstance()) return false;
	jobject typeOfHit = Java::Env->GetObjectField(this->GetInstance(), StrayCache::movingObjectPosition_typeOfHit);
	if (!typeOfHit || Java::Env->ExceptionCheck()) return false;

	jclass movingObjectType = Java::Env->GetObjectClass(typeOfHit);
	if (!movingObjectType || Java::Env->ExceptionCheck()) return false;
	jfieldID block = StrayCache::GetMovingBlockPositionBLOCK(movingObjectType);
	if (!block || Java::Env->ExceptionCheck()) return false;
	jobject object = Java::Env->GetStaticObjectField(movingObjectType, block);
	if (!object || Java::Env->ExceptionCheck()) return false;

	return Java::Env->IsSameObject(object, typeOfHit);
}

CEntity CMovingObjectPosition::GetEntityHit()
{
	jobject typeOfHit = Java::Env->GetObjectField(this->GetInstance(), StrayCache::movingObjectPosition_typeOfHit);
	if (!typeOfHit) return CEntity();

	return CEntity(Java::Env->GetObjectField(this->GetInstance(), StrayCache::movingObjectPosition_entityHit));
}

