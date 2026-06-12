#include "Pool/PooledObject.h"
#include "Pool/MyObjectPoolSubsystem.h"

void UPooledObject::Init(UMyObjectPoolSubsystem* Owner)
{
	bIsPoolActive = false;
	ObjectPool = Owner;
}

void UPooledObject::RecycleSelf()
{
	if (ObjectPool)
	{
		ObjectPool->RecyclePooledObject(this);
	}
}

void UPooledObject::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	//Init 없이 붙은 컴포넌트(BP에서 수동 추가 등)는 ObjectPool이 비어있을 수 있음
	if (ObjectPool)
	{
		ObjectPool->OnPoolerCleanup.RemoveDynamic(this, &UPooledObject::RecycleSelf);
	}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}