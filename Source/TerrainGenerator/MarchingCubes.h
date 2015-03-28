#pragma once
#include "TerrainGenerator.h"
#include "DynamicMeshBuilder.h"



/**
 * Utility Class for extracting Iso Surfaces
 */


class UMarchingCubes
{
private:
	FIntVector GridSize;
	float ***m_pVoxels;
	float m_fSurfaceCrossValue;
public:
	UMarchingCubes();
	~UMarchingCubes();

	// Returns the number of triangles generated.
	int PolygonizeToTriangles(TArray<FDynamicMeshVertex> *Vertices, TArray<int32> *Indices, TArray<FVector> *Positions, float fScaling, int32 SizeX, int32 SizeY, int32 SizeZ, int32 PosX, int32 PosY, int32 PosZ);

	void CreateGrid(int32 SizeX, int32 SizeY, int32 SizeZ, float InitialIsoValue = 0.0f);
	void ClearGrid(float fValue);
	void DestroyGrid();
	void SetSurfaceCrossOverValue(float fValue);
	float GetSurfaceCrossOverValue();
	float GetVoxel(int32 X, int32 Y, int32 Z);
	void SetVoxel(int32 X, int32 Y, int32 Z, float IsoValue);
	FIntVector GetGridSize();
};

