// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainGenerator.h"
#include "ProceduralTerrain.h"


AProceduralTerrain::AProceduralTerrain(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create Root Component
	SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
	RootComponent = SceneRoot;
	
	TerrainGenerationWorker = 0;
	//PrimaryActorTick.bCanEverTick = true;
	gMaterial = NULL;
	WaitingThreads.Init(0);
	MaxThreads = 1;
}

bool AProceduralTerrain::GenerateFromOrigin(int32 X, int32 Y, int32 Z, int32 Size)
{
	int32 StartX = X - Size;
	int32 StartY = Y - Size;
	int32 EndX = X + Size;
	int32 EndY = Y + Size;

	// Clear X Axis
	for (int32 tY = StartY; tY <= EndY; ++tY)
	{
		DestroyChunk(StartX - 1, tY, Z);
		DestroyChunk(EndX + 1, tY, Z);
	}

	// Clear Y Axis
	for (int32 tX = StartX; tX <= EndX; ++tX)
	{
		DestroyChunk(tX, StartY - 1, Z); 
		DestroyChunk(tX, EndY + 1, Z);
	}

	// Generate Chunks along X & Y Axes
	for (int32 tX = StartX; tX < EndX; ++tX)
	{
		for (int32 tY = StartY; tY < EndY; ++tY)
		{
			CreateChunk(tX, tY, Z);
		}
	}
	return true;
}


bool AProceduralTerrain::ToggleCollision(int32 X, int32 Y, int32 Z, bool collide)
{


	// For memory saving purposes?!
	int32 ComponentNum = TerrainMeshComponents.Num();
	for (int i = 0; i < ComponentNum; ++i)
	{
		if (
			TerrainMeshComponents[i]->WorldPosition.X == X
			&&
			TerrainMeshComponents[i]->WorldPosition.Y == Y
			&&
			TerrainMeshComponents[i]->WorldPosition.Z == Z
			)
		{
			if (collide)
			{
				TerrainMeshComponents[i]->UpdateCollision();
				return true;
			}
		}
		TerrainMeshComponents[i]->RemoveCollision();

	}
	return false;
}



bool AProceduralTerrain::CreateChunk(int32 X, int32 Y, int32 Z)
{
	// Make sure we don't create duplicated chunks
	int32 ComponentNum = TerrainMeshComponents.Num();
	for (int i = 0; i < ComponentNum; ++i)
	{
		if (
			TerrainMeshComponents[i]->WorldPosition.X == X
			&&
			TerrainMeshComponents[i]->WorldPosition.Y == Y
			&&
			TerrainMeshComponents[i]->WorldPosition.Z == Z
			)
		{
			return false;
		}
	}

	
	
	// Create a Thread if we did n't hyet
	if (!TerrainGenerationWorker)
	{
		TerrainGenerationWorker = new FTerrainGenerationWorker();
		// Let's go! 
		TerrainGenerationWorker->Start();
	}

	



	UTerrainMeshComponent *MeshComponent = CreateTerrainComponent();
	MeshComponent->WorldPosition.X = X;
	MeshComponent->WorldPosition.Y = Y;
	MeshComponent->WorldPosition.Z = Z;

	FTerrainChunk Chunk;
	Chunk.MeshComponent = MeshComponent;
	Chunk.IsChunkGenerated = false;
	Chunk.XPos = X;
	Chunk.YPos = Y;
	Chunk.ZPos = Z;
	
	TerrainGenerationWorker->VerticalSmoothing = VerticalSmoothness;
	TerrainGenerationWorker->VerticalScaling = VerticalScaling;
	TerrainGenerationWorker->Scale = Scale;
	TerrainGenerationWorker->Width = ChunkWidth;
	TerrainGenerationWorker->Length = ChunkLength;
	TerrainGenerationWorker->Height = ChunkHeight;

	TerrainGenerationWorker->CaveScaleA = CaveScaleA;
	TerrainGenerationWorker->CaveScaleB = CaveScaleB;
	TerrainGenerationWorker->CaveDensityAmplitude = CaveDensityAmplitude;
	TerrainGenerationWorker->CaveModA = CaveModA;
	TerrainGenerationWorker->CaveModB = CaveModB;
	TerrainGenerationWorker->CaveModC = CaveModC;
	TerrainGenerationWorker->CaveModD = CaveModD;

	TerrainGenerationWorker->Ground = Ground;
	TerrainGenerationWorker->MarchingCubes->SetSurfaceCrossOverValue(SurfaceCrossOverValue);

	TerrainGenerationWorker->ChunkTasks.Add(Chunk);

	UE_LOG(LogClass, Log, TEXT("ChunkTasks: %d"), TerrainGenerationWorker->ChunkTasks.Num());


	TerrainMeshComponents.Add(MeshComponent);

	return true;
}

bool AProceduralTerrain::DestroyChunk(int32 X, int32 Y, int32 Z)
{
	int32 ComponentNum = TerrainMeshComponents.Num();
	for (int i = 0; i < ComponentNum; ++i)
	{
		if (
			TerrainMeshComponents[i]->WorldPosition.X == X
			&&
			TerrainMeshComponents[i]->WorldPosition.Y == Y
			&&
			TerrainMeshComponents[i]->WorldPosition.Z == Z
			)
		{
			TerrainMeshComponents[i]->UnregisterComponent();
			TerrainMeshComponents[i]->DestroyComponent();
			TerrainMeshComponents.RemoveAt(i);
			return true;
		}

	}
	return false;
}

UTerrainMeshComponent * AProceduralTerrain::CreateTerrainComponent()
{
	// Generate different names for our component to supress warnings
	FString ComponentName;
	int32 ID = TerrainMeshComponents.Num();
	ComponentName.Append(TEXT("TerrainMeshComponent"));
	ComponentName.AppendInt(ID);
	FName name;
	name.AppendString(ComponentName);

	// Create our TerrainMeshComponent 
	UTerrainMeshComponent *MeshComponent = ConstructObject<UTerrainMeshComponent>(UTerrainMeshComponent::StaticClass(), this, name);
	MeshComponent->RegisterComponent();

	// Apply a material if we have any
	if (!gMaterial)
		gMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
	MeshComponent->SetMaterial(0, gMaterial);

	return MeshComponent;
}

void AProceduralTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AProceduralTerrain::UpdateTerrain()
{
	if (TerrainGenerationWorker != 0)
	{
		int32 TaskNum = TerrainGenerationWorker->ChunkTasks.Num();
		for (int32 i = 0; i < TaskNum; ++i)
		{
			if (TerrainGenerationWorker->ChunkTasks[i].IsChunkGenerated)
			{

				TerrainGenerationWorker->ChunkTasks[i].MeshComponent->Positions = TerrainGenerationWorker->ChunkTasks[i].Positions;
				TerrainGenerationWorker->ChunkTasks[i].MeshComponent->Indices = TerrainGenerationWorker->ChunkTasks[i].Indices;
				TerrainGenerationWorker->ChunkTasks[i].MeshComponent->Vertices = TerrainGenerationWorker->ChunkTasks[i].Vertices;
				// Update the Mesh Component
				TerrainGenerationWorker->ChunkTasks[i].MeshComponent->MarkRenderable(true);
				TerrainGenerationWorker->ChunkTasks[i].MeshComponent->UpdateCollision();
				TerrainGenerationWorker->ChunkTasks.RemoveAt(i);


		
				return true;
			}
		}
	}
	return false;
}

void AProceduralTerrain::BeginDestroy()
{
	// Destroy the thread
	if (TerrainGenerationWorker != 0)
	{
		TerrainGenerationWorker->Stop();
		delete TerrainGenerationWorker;
	}
	Super::BeginDestroy();
}
