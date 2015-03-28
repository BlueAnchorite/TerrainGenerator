#pragma once
#include "TerrainGenerator.h"
#include "MarchingCubes.h"
#include "TerrainMeshComponent.h"
#include "GenericPlatformProcess.h"

struct FTerrainChunk
{
	int32 XPos;
	int32 YPos;
	int32 ZPos;

	TArray<FVector> Positions;
	TArray<FDynamicMeshVertex> Vertices;
	TArray<int32> Indices;

	UTerrainMeshComponent *MeshComponent;

	FTerrainChunk()
	{
		MeshComponent = 0;


	}
};

class FTerrainGenerationWorker : public FRunnable
{	
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
	bool bIsRunning;
	FThreadSafeCounter StopTaskCounter;
	UMarchingCubes *MarchingCubes;
public:


	// Generation Parameters
	int32 Width;
	int32 Length;
	int32 Height;

	int32 Ground;

	float Scale;
	float VerticalScaling;
	float VerticalSmoothing;

	float CaveScaleA;
	float CaveScaleB;
	float CaveDensityAmplitude;

	float CaveModA;
	float CaveModB;

	float SurfaceCrossOverValue;
	
	TQueue <FTerrainChunk> QueuedChunks;
	TQueue <FTerrainChunk> FinishedChunks;

	static int32 ThreadCount;

	bool IsRunning() const
	{
		return bIsRunning;
	};
	

	FTerrainGenerationWorker();
	virtual ~FTerrainGenerationWorker();
 
	bool Start();
	virtual bool Init() override;
	virtual uint32 Run();
	virtual void Stop() override;
	virtual void Exit() override;
	void Shutdown();

	bool GenerateChunk(FTerrainChunk &chunk);

 
	void EnsureCompletion();
 
 
};