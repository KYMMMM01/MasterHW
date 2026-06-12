#pragma once

#include "CoreMinimal.h"
#include "PooledObjectData.generated.h"

USTRUCT(BlueprintType)
struct FPooledObjectData
{
	GENERATED_BODY()

	FPooledObjectData()
	{
		ActorTemplate = nullptr;
		PoolSize = 1;
		bCanGrow = false;
		ActorName = "default";
	}

	//어떤 액터를 스폰할 것인가.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorTemplate;

	//몇개를 스폰할 것인가
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PoolSize;

	//없으면 추가로 생성할 예정인가
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanGrow;

	//액터의 이름은 무엇인가
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActorName;

};
