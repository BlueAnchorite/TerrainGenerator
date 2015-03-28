#include "TerrainGenerator.h"
#include "TerrainGenerationWorker.h"
#include "Noise.h"

int32 FTerrainGenerationWorker::ThreadCount = 0;

FTerrainGenerationWorker::FTerrainGenerationWorker()
	: StopTaskCounter(0),
	Thread(0),
	bIsRunning(false),
	SurfaceCrossOverValue(0.0f)
{

}

bool FTerrainGenerationWorker::Start()
{
	if (FPlatformProcess::SupportsMultithreading())
	{
		Thread = FRunnableThread::Create(this, TEXT("FTerrainGenerationWorker"), 0, TPri_AboveNormal); //windows default = 8mb for thread, could specify more
		return true;
	}
	else{
		return false;
	}
}



 
bool FTerrainGenerationWorker::GenerateChunk(FTerrainChunk &Chunk)
{

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
				float Density = (UNoise::MakeSimplexNoise2D(tXPos + x + z, tYPos + y, CaveScaleA) + CaveModA) - (UNoise::MakeSimplexNoise2D(tXPos + x, tYPos + y + z, CaveScaleB) - CaveModB);
				Density += CaveDensityAmplitude;
				MarchingCubes->SetVoxel(x, y, z, Density);
			}
		}
	}

	// Polygonize!
	MarchingCubes->PolygonizeToTriangles(&Chunk.Vertices, &Chunk.Indices, &Chunk.Positions, Scale, Width, Length, Height, tXPos, tYPos, tZPos);
	return true;
	
}

bool FTerrainGenerationWorker::Init()
{
	++FTerrainGenerationWorker::ThreadCount;

	MarchingCubes = new UMarchingCubes();
	MarchingCubes->SetSurfaceCrossOverValue(SurfaceCrossOverValue);
	return true;
}

uint32 FTerrainGenerationWorker::Run()
{
	bIsRunning = true;
	while (StopTaskCounter.GetValue() == 0 && bIsRunning)
	{
		if (!QueuedChunks.IsEmpty())
		{
			FTerrainChunk Chunk;
			QueuedChunks.Dequeue(Chunk);

			if (this->GenerateChunk(Chunk))
			{
				FinishedChunks.Enqueue(Chunk);
			}
			
			
		}
		FPlatformProcess::Sleep(0.03);
	}
	bIsRunning = false;

	return 0;
}

void FTerrainGenerationWorker::Exit()
{
	delete MarchingCubes;
	MarchingCubes = 0;

	--FTerrainGenerationWorker::ThreadCount;
}
 
void FTerrainGenerationWorker::EnsureCompletion()
{
	if (!Thread)
		return;
	Stop();
	Thread->WaitForCompletion();
}

void FTerrainGenerationWorker::Stop()
{
	if (!Thread)
		return;

	StopTaskCounter.Increment();
}




void FTerrainGenerationWorker::Shutdown()
{
	if (!Thread)
		return;
	Thread->Kill(true);
}
 


FTerrainGenerationWorker::~FTerrainGenerationWorker()
{
	delete Thread;
	Thread = NULL;
	delete MarchingCubes;
	MarchingCubes = NULL;
}
