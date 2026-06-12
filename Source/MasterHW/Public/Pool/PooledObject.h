#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PooledObject.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MASTERHW_API UPooledObject : public UActorComponent
{
	GENERATED_BODY()

public:

	void Init(class UMyObjectPoolSubsystem* Owner);

	UFUNCTION(BlueprintCallable)
	void RecycleSelf();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bIsPoolActive;

private:

	TObjectPtr<class UMyObjectPoolSubsystem> ObjectPool;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
};
