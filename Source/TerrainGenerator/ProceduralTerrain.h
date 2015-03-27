// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TerrainMeshComponent.h"
#include "MarchingCubes.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerationWorker.h"
#include "ProceduralTerrain.generated.h"

/**
 * 
 */





UCLASS()
class TERRAINGENERATOR_API AProceduralTerrain : public AActor
{
	GENERATED_BODY()
private:

	FTerrainGenerationWorker *TerrainGenerationWorker;

	class USceneComponent* SceneRoot;

public:

	/*
	*	PROPERTIES
	*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	int32 ChunkWidth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	int32 ChunkLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	int32 ChunkHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	int32 Ground;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation|Performance")
	int32 MaxThreads;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation|Performance")
	TArray<FIntVector> WaitingThreads;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float SurfaceCrossOverValue;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float VerticalSmoothness;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float VerticalScaling;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float Scale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveScaleA;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveScaleB;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveDensityAmplitude;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveModA;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveModB;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveModC;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	float CaveModD;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain Generation")
	UMaterialInterface *gMaterial;

	

	AProceduralTerrain(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool GenerateFromOrigin(int32 X, int32 Y, int32 Z, int32 Size);

	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool ToggleCollision(int32 X, int32 Y, int32 Z, bool collide);

	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool CreateChunk(int32 X, int32 Y, int32 Z);

	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool DestroyChunk(int32 X, int32 Y, int32 Z);

	

	

	// Returns true if the terrain was updated
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool UpdateTerrain();
	
	virtual void Tick(float DeltaTime) override;

	TArray<class UTerrainMeshComponent *> TerrainMeshComponents;

	virtual void BeginDestroy() override;
private:
	UTerrainMeshComponent *CreateTerrainComponent();
};
