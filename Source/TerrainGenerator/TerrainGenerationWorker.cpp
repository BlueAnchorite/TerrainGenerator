#include "TerrainGenerator.h"
#include "TerrainGenerationWorker.h"
#include "Noise.h"

int32 FTerrainGenerationWorker::ThreadCount = 0;

FTerrainGenerationWorker::FTerrainGenerationWorker()
	: StopTaskCounter(0)
{
	MarchingCubes = ConstructObject<UMarchingCubes>(UMarchingCubes::StaticClass());
	ChunkTasks.Init(0);
	bIsRunning = false;
}

bool FTerrainGenerationWorker::Start()
{
	if (FPlatformProcess::SupportsMultithreading())
	{
		bIsRunning = false;
		Thread = FRunnableThread::Create(this, TEXT("FTerrainGenerationWorker"), 0, TPri_AboveNormal); //windows default = 8mb for thread, could specify more
		return true;
	}
	else{
		bIsRunning = true;
		return false;
	}
}

FTerrainGenerationWorker::~FTerrainGenerationWorker()
{
	delete Thread;
	Thread = NULL;
	delete MarchingCubes;
	MarchingCubes = NULL;
}


void FTerrainGenerationWorker::Stop()
{
	if (Thread)
	{
		StopTaskCounter.Increment();
		
		Thread->WaitForCompletion();

	}
}
 
void FTerrainGenerationWorker::GenerateChunk(FTerrainChunk &Chunk)
{
	if (Chunk.IsChunkGenerated)
		return;


	int32 tXPos = Chunk.XPos * (Width - 1);
	int32 tYPos = Chunk.YPos * (Length - 1);
	int32 tZPos = Chunk.ZPos * (Height - 1);


	// Create our Grid (The smaller the grid is the faster the less work our thread has to do.)
	MarchingCubes->CreateGrid(Width, Length, Height, 1.0f);

	// Hills
	for (int32 x = 0; x < Width; ++x)
	{
		for (int32 y = 0; y < Length; ++y)
		{
			float zer = 0.0f;

			// Simplex Noise Height map
			float Density = UNoise::MakeSimplexNoise2D(tXPos + x, tYPos + y, VerticalScaling);

			//Density -= FMath::Sin(((float)y) * VerticalScaling);
			for (int32 z = Ground; z <= Height; ++z)
			{
				float tmp = Density + ((float)zer / Height);
				MarchingCubes->SetVoxel(x, y, z, tmp);
				zer += VerticalSmoothing;
			}

		}
	}

	// Ground
	for (int32 x = 0; x < Width; ++x)
	{
		for (int32 y = 0; y < Length; ++y)
		{
			for (int32 z = 0; z < Ground; ++z)
			{
				MarchingCubes->SetVoxel(x, y, z, -1.0f);
			}
		}

	}

	// Cave things
	for (int32 x = 0; x < Width; ++x)
	{
		for (int32 y = 0; y < Length; ++y)
		{
			for (int32 z = 0; z <= Ground; ++z)
			{
				//float Density = UNoise::MakeOctaveNoise3D(CaveOctaves, CavePersistence, CaveScale, (float)x*SimplexScale, (float)y*SimplexScale, (float)z*SimplexScale);
				float Density = UNoise::MakeSimplexNoise2D(tXPos + x + z*CaveModA, tYPos + y*CaveModB, CaveScaleA) - UNoise::MakeSimplexNoise2D(tXPos + x*CaveModC, tYPos + y + z*CaveModD, CaveScaleB);
				Density += CaveDensityAmplitude;
				MarchingCubes->SetVoxel(x, y, z, Density);
			}
		}
	}

	// Polygonize!
	MarchingCubes->PolygonizeToTriangles(&Chunk.Vertices, &Chunk.Indices, &Chunk.Positions, Scale, Width, Length, Height, tXPos, tYPos, tZPos);

	Chunk.IsChunkGenerated = true;
}

uint32 FTerrainGenerationWorker::Run()
{
	++FTerrainGenerationWorker::ThreadCount;
	while (StopTaskCounter.GetValue() == 0)
	{
		if (ChunkTasks.Num() > 0)
		{
			GenerateChunk(ChunkTasks[0]);
		}
		FPlatformProcess::Sleep(0.05);
	}

	bIsRunning = false;
	--FTerrainGenerationWorker::ThreadCount;
	return 0;
}
 
void FTerrainGenerationWorker::EnsureCompletion()
{
	Thread->WaitForCompletion();
}
 