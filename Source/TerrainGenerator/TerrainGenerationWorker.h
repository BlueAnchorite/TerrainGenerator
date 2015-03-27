#pragma once
#include "TerrainGenerator.h"
#include "MarchingCubes.h"
#include "TerrainMeshComponent.h"
#include "TerrainGenerationWorker.generated.h"

USTRUCT()
struct FTerrainChunk
{
	GENERATED_USTRUCT_BODY()


	bool IsChunkGenerated;

	int32 XPos;
	int32 YPos;
	int32 ZPos;

	TArray<FVector> Positions;
	TArray<FDynamicMeshVertex> Vertices;
	TArray<int32> Indices;

	UTerrainMeshComponent *MeshComponent;

	FTerrainChunk()
	{
		IsChunkGenerated = false;
		MeshComponent = 0;


	}
};

class FTerrainGenerationWorker : public FRunnable
{	
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
	bool bIsRunning;
	FThreadSafeCounter StopTaskCounter;
public:



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
	float CaveModC;
	float CaveModD;

	TArray<FTerrainChunk> ChunkTasks;

	UMarchingCubes *MarchingCubes;
	static int32 ThreadCount;

	bool IsRunning() const
	{
		return bIsRunning;
	};
	

	FTerrainGenerationWorker();
	virtual ~FTerrainGenerationWorker();
 
	bool Start();

	virtual uint32 Run();
	virtual void Stop();

	void GenerateChunk(FTerrainChunk &chunk);

 
	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();
 
 
};