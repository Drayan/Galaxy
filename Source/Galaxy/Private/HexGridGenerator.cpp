// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridGenerator.h"
#include "HexGridAsset.h"
#include "HexCell.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

void FTriangleMesh::Clear()
{
	Vertices.Empty();
	Indices.Empty();
	VertexToTriangleMap.Empty();
	TriangleNeighbors.Empty();
}

void FTriangleMesh::GetTriangle(int32 triIdx, int32& outV0, int32& outV1, int32& outV2) const
{
	int32 baseIdx = triIdx * 3;
	outV0 = Indices[baseIdx];
	outV1 = Indices[baseIdx + 1];
	outV2 = Indices[baseIdx + 2];
}

FVector FTriangleMesh::GetTriangleCenter(int32 triIdx) const
{
	int32 v0, v1, v2;
	GetTriangle(triIdx, v0, v1, v2);
	FVector center = (Vertices[v0] + Vertices[v1] + Vertices[v2]) / 3.0f;
	return center.GetSafeNormal();
}

UHexGridAsset* UHexGridGenerator::GenerateHexGrid(int32 level, TArray<FString>& OutErrors)
{
	OutErrors.Empty();

	if (level < 0 || level > 10)
	{
		OutErrors.Add(FString::Printf(TEXT("Invalid level %d. Level must be between 0 and 10."), level));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Starting generation for level %d."), level);

	// Create the asset
	UHexGridAsset* hexGrid = NewObject<UHexGridAsset>();
	hexGrid->GridLevel = level;

	// Step 1 : Create base icosahedron
	FTriangleMesh mesh;
	CreateIcosahedron(mesh);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Created base icosahedron with %d vertices and %d triangles."), mesh.Vertices.Num(), mesh.GetTriangleCount());

	// Step 2 : Subdivide mesh
	SubdivideMesh(mesh, level);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Subdivided mesh to level %d with %d vertices and %d triangles."), level, mesh.Vertices.Num(), mesh.GetTriangleCount());

	// Step 3 : Build adjacency data
	BuildAdjacencyData(mesh);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Built adjacency data."));

	// Step 4 : Generate hex grid from triangle mesh
	ConvertToHexDual(mesh, hexGrid);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Converted to hex dual grid with %d cells."), hexGrid->Cells.Num());

	// Step 5 : Build cell neighbors
	BuildCellNeighbors(hexGrid);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Built cell neighbors."));

	// Step 6 : Order cell vertices
	OrderCellVertices(hexGrid);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Ordered cell vertices."));

	// Step 7 : Assign icosahedron faces
	AssignIcosahedronFaces(hexGrid);
	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Assigned icosahedron faces."));

	// Calculate statistics
	hexGrid->CalculateStatistics();

	// Validate
	TArray<FString> validationErrors;
	bool bValid = hexGrid->ValidateGrid(validationErrors);

	if (!bValid)
	{
		OutErrors.Append(validationErrors);
		UE_LOG(LogTemp, Error, TEXT("HexGridGenerator: Validation failed with %d errors."), validationErrors.Num());
	}
	{
		UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Validation succeeded."));
	}

	return hexGrid;
}

UHexGridAsset* UHexGridGenerator::CreateHexGridAsset(int32 level, const FString& assetName, const FString& assetPath, TArray<FString>& OutErrors)
{
	// Generate the grid
	UHexGridAsset* hexGrid = GenerateHexGrid(level, OutErrors);
	if (!hexGrid || OutErrors.Num() > 0)
	{
		return nullptr;
	}

#if WITH_EDITOR
	// Create package
	FString packageName = assetPath + assetName;
	UPackage* package = CreatePackage(*packageName);

	if (!package)
	{
		OutErrors.Add(TEXT("Failed to create package."));
		return nullptr;
	}

	// Rename the grid to the package
	hexGrid->Rename(*assetName, package);

	// Mark package dirty
	package->MarkPackageDirty();

	// Save the package
	FSavePackageArgs saveArgs;
	saveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	saveArgs.SaveFlags = SAVE_NoError;
	
	FString packageFileName = FPackageName::LongPackageNameToFilename(packageName, FPackageName::GetAssetPackageExtension());

	if (!UPackage::SavePackage(package, hexGrid, *packageFileName, saveArgs))
	{
		OutErrors.Add(TEXT("Failed to save package."));
		return nullptr;
	}

	// Notify asset registry
	FAssetRegistryModule::AssetCreated(hexGrid);

	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Created and saved hex grid asset '%s'."), *packageName);
#endif

	return hexGrid;
}

void UHexGridGenerator::CreateIcosahedron(FTriangleMesh& outMesh)
{
	outMesh.Clear();

	// Golden ratio
	const float phi = (1.0f + FMath::Sqrt(5.0f)) * 0.5f;
	const float invNorm = 1.0f / FMath::Sqrt(1.0f + phi * phi);

	// 12 vertices of an icosahedron (on unit sphere)
	TArray<FVector> vertices = {
		FVector(-1,  phi, 0) * invNorm,
		FVector( 1,  phi, 0) * invNorm,
		FVector(-1, -phi, 0) * invNorm,
		FVector( 1, -phi, 0) * invNorm,

		FVector(0, -1,  phi) * invNorm,
		FVector(0,  1,  phi) * invNorm,
		FVector(0, -1, -phi) * invNorm,
		FVector(0,  1, -phi) * invNorm,

		FVector( phi, 0, -1) * invNorm,
		FVector( phi, 0,  1) * invNorm,
		FVector(-phi, 0, -1) * invNorm,
		FVector(-phi, 0,  1) * invNorm
	};

	outMesh.Vertices = MoveTemp(vertices);

	// 20 triangular faces of the icosahedron
	TArray<int32> indices = {
		// 5 faces around point 0
		0, 11, 5,
		0, 5, 1,
		0, 1, 7,
		0, 7, 10,
		0, 10, 11,

		// 5 adjacent faces
		1, 5, 9,
		5, 11, 4,
		11, 10, 2,
		10, 7, 6,
		7, 1, 8,

		// 5 faces around point 3
		3, 9, 4,
		3, 4, 2,
		3, 2, 6,
		3, 6, 8,
		3, 8, 9,

		// 5 adjacent faces
		4, 9, 5,
		2, 4, 11,
		6, 2, 10,
		8, 6, 7,
		9, 8, 1
	};

	outMesh.Indices = MoveTemp(indices);
}

void UHexGridGenerator::SubdivideMesh(FTriangleMesh& mesh, int32 subdivisions)
{
	for (int32 level = 0; level < subdivisions; ++level)
	{
		TArray<int32> oldIndices = mesh.Indices;
		mesh.Indices.Empty();

		int32 numTriangles = oldIndices.Num() / 3;

		for (int32 triIdx = 0; triIdx < numTriangles; ++triIdx)
		{
			int32 v0 = oldIndices[triIdx * 3 + 0];
			int32 v1 = oldIndices[triIdx * 3 + 1];
			int32 v2 = oldIndices[triIdx * 3 + 2];

			TArray<int32> newIndices;
			SubdivideTriangle(mesh, v0, v1, v2, newIndices);

			mesh.Indices.Append(newIndices);
		}
	}
}

void UHexGridGenerator::SubdivideTriangle(FTriangleMesh& mesh, int32 v0, int32 v1, int32 v2, TArray<int32>& outNewVertices)
{
	// Find or create midpoints vertices
	FVector p0 = mesh.Vertices[v0];
	FVector p1 = mesh.Vertices[v1];
	FVector	p2 = mesh.Vertices[v2];

	FVector m01 = ((p0 + p1) * 0.5f).GetSafeNormal();
	FVector m12 = ((p1 + p2) * 0.5f).GetSafeNormal();
	FVector m02 = ((p2 + p0) * 0.5f).GetSafeNormal();

	int32 m01Idx = GetOrAddVertex(mesh, m01);
	int32 m12Idx = GetOrAddVertex(mesh, m12);
	int32 m02Idx = GetOrAddVertex(mesh, m02);

	// Create 4 new triangles
	outNewVertices = {
		v0,   m01Idx, m02Idx,   // Top
		m01Idx, v1,   m12Idx,   // Left
		m02Idx, m12Idx, v2,     // Right
		m01Idx, m12Idx, m02Idx  // Center
	};
}

int32 UHexGridGenerator::GetOrAddVertex(FTriangleMesh& mesh, FVector position, float mergeThreshold /* = 0.0001f */)
{
	FVector normPos = position.GetSafeNormal();

	// Check if vertex already exists (within threshold)
	for (int32 i = 0; i < mesh.Vertices.Num(); ++i)
	{
		if (PositionsEqual(mesh.Vertices[i], normPos, mergeThreshold))
		{
			return i;
		}
	}

	// Add new vertex
	int32 newIdx = mesh.Vertices.Add(normPos);
	return newIdx;
}

void UHexGridGenerator::BuildAdjacencyData(FTriangleMesh& mesh)
{
	mesh.VertexToTriangleMap.Empty();
	mesh.TriangleNeighbors.Empty();

	int32 numTriangles = mesh.GetTriangleCount();

	// Build vertex -> triangle map
	for (int32 triIdx = 0; triIdx < numTriangles; ++triIdx)
	{
		int32 v0, v1, v2;
		mesh.GetTriangle(triIdx, v0, v1, v2);

		mesh.VertexToTriangleMap.FindOrAdd(v0).AddUnique(triIdx);
		mesh.VertexToTriangleMap.FindOrAdd(v1).AddUnique(triIdx);
		mesh.VertexToTriangleMap.FindOrAdd(v2).AddUnique(triIdx);
	}

	// Build triangle neighbors (triangles sharing an edge)
	mesh.TriangleNeighbors.SetNum(numTriangles);

	for (int32 triIdx = 0; triIdx < numTriangles; ++triIdx)
	{
		int32 v0, v1, v2;
		mesh.GetTriangle(triIdx, v0, v1, v2);

		TArray<int32>& neighbors = mesh.TriangleNeighbors[triIdx];

		// Find neighbors by shared edges
		auto FindNeighbor = [&](int32 vA, int32 vB)
			{
				const TArray<int32>* trianglesA = mesh.VertexToTriangleMap.Find(vA);
				const TArray<int32>* trianglesB = mesh.VertexToTriangleMap.Find(vB);

				if (trianglesA && trianglesB)
				{
					for (int32 T : *trianglesA)
					{
						if (T != triIdx && trianglesB->Contains(T))
						{
							neighbors.AddUnique(T);
							break;
						}
					}
				}
			};

		FindNeighbor(v0, v1);
		FindNeighbor(v1, v2);
		FindNeighbor(v2, v0);
	}
}

void UHexGridGenerator::ConvertToHexDual(const FTriangleMesh& triMesh, UHexGridAsset* outGrid)
{
	outGrid->Cells.Empty();
	outGrid->PentagonCellsIds.Empty();

	int32 numVertices = triMesh.Vertices.Num();
	outGrid->Cells.SetNum(numVertices);

	// Each vertex in the triangle mesh becomes a hex cell in the hex grid
	for (int32 vertIdx = 0; vertIdx < numVertices; ++vertIdx)
	{
		FHexCell& cell = outGrid->Cells[vertIdx];
		cell.CellId = vertIdx;
		cell.Position = triMesh.Vertices[vertIdx];

		// Find all triangles that use this vertex
		const TArray<int32>* triangles = triMesh.VertexToTriangleMap.Find(vertIdx);

		if (!triangles)
		{
			continue;
		}

		int32 numTriangles = triangles->Num();

		// Type: 5 triangles = pentagon, 6 triangles = hexagon
		if (numTriangles == 5)
		{
			cell.CellType = EHexCellType::Pentagon;
			outGrid->PentagonCellsIds.Add(vertIdx);
		}
		else if (numTriangles == 6)
		{
			cell.CellType = EHexCellType::Hexagon;
		}
		else
		{
			// Unexpected, but handle gracefully
			UE_LOG(LogTemp, Warning, TEXT("ConvertToHexDual: Vertex %d has %d triangles, expected 5 or 6."), vertIdx, numTriangles);
		}

		// The vertices of this cell are the centers of the adjacent triangles
		cell.Vertices.Empty();
		for (int32 triIdx : *triangles)
		{
			FVector triCenter = triMesh.GetTriangleCenter(triIdx);
			cell.Vertices.Add(triCenter);
		}
	}

	// Set counts
	outGrid->TotalCellCount = outGrid->Cells.Num();
	outGrid->PentagonCount = outGrid->PentagonCellsIds.Num();
	outGrid->HexagonCount = outGrid->TotalCellCount - outGrid->PentagonCount;

	UE_LOG(LogTemp, Log, TEXT("   - Hex dual : %d hexagons, %d pentagons."), outGrid->HexagonCount, outGrid->PentagonCount);
}

void UHexGridGenerator::BuildCellNeighbors(UHexGridAsset* grid)
{
	// For each cell, find neighbors by checking which cells share edge vertices
	for (int32 cellId = 0; cellId < grid->Cells.Num(); ++cellId)
	{
		FHexCell& cell = grid->Cells[cellId];
		cell.NeighborCellIds.Empty();

		// Neighbors are cells that share at least 2 vertex positions (an edge)
		for (int32 otherId = 0; otherId < grid->Cells.Num(); ++otherId)
		{
			if (otherId == cellId)
			{
				continue;
			}

			const FHexCell& otherCell = grid->Cells[otherId];

			// Count shared vertices
			int32 sharedCount = 0;
			for (const FVector& v1 : cell.Vertices)
			{
				for (const FVector& v2 : otherCell.Vertices)
				{
					if (PositionsEqual(v1, v2))
					{
						sharedCount++;
						break;
					}
				}
			}

			// If they share 2 vertices, they are neighbors
			if (sharedCount >= 2)
			{
				cell.NeighborCellIds.Add(otherId);
			}
		}

		// Verify neighbor count
		int32 expectedNeighbors = cell.GetNeighborCount();
		if (cell.NeighborCellIds.Num() != expectedNeighbors)
		{
			UE_LOG(LogTemp, Warning, TEXT("BuildCellNeighbors: Cell %d has %d neighbors, expected %d."), cellId, cell.NeighborCellIds.Num(), expectedNeighbors);
		}
	}
}

void UHexGridGenerator::OrderCellVertices(UHexGridAsset* grid)
{
	// Order vertices counter-clockwise around cell center
	for (FHexCell& cell : grid->Cells)
	{
		OrderVerticesCounterClockwise(cell.Position, cell.Vertices);
	}
}

void UHexGridGenerator::OrderVerticesCounterClockwise(const FVector& center, TArray<FVector>& vertices)
{
	if (vertices.Num() < 3)
	{
		return;
	}

	// Use the first vertex as reference
	FVector reference = vertices[0];

	// Create a local coordinate system
	FVector normal = center.GetSafeNormal();
	FVector tangent = (reference - center * FVector::DotProduct(reference, center)).GetSafeNormal();
	FVector bitangent = FVector::CrossProduct(normal, tangent);

	// Sort by angle around center
	vertices.Sort([&](const FVector& A, const FVector& B)
		{
			FVector vA = A - center * FVector::DotProduct(A, center);
			FVector vB = B - center * FVector::DotProduct(B, center);

			float angleA = FMath::Atan2(FVector::DotProduct(vA, bitangent), FVector::DotProduct(vA, tangent));
			float angleB = FMath::Atan2(FVector::DotProduct(vB, bitangent), FVector::DotProduct(vB, tangent));

			return angleA < angleB;
		});
}

void UHexGridGenerator::AssignIcosahedronFaces(UHexGridAsset* grid)
{
	// Create the base icosahedron faces
	FTriangleMesh icosahedron;
	CreateIcosahedron(icosahedron);

	// Calculate center of each icosahedron face
	TArray<FVector> faceCenters;
	for (int32 faceIdx = 0; faceIdx < 20; ++faceIdx)
	{
		FVector center = icosahedron.GetTriangleCenter(faceIdx);
		faceCenters.Add(center);
	}

	// Assign each cell to the closest icosahedron face
	for (FHexCell& cell : grid->Cells)
	{
		float minDist = FLT_MAX;
		int32 closestFace = 0;

		for (int32 faceIdx = 0; faceIdx < faceCenters.Num(); ++faceIdx)
		{
			float dist = FVector::DistSquared(cell.Position, faceCenters[faceIdx]);
			if (dist < minDist)
			{
				minDist = dist;
				closestFace = faceIdx;
			}
		}

		cell.IcosaheronFaceIndex = static_cast<uint8>(closestFace);
	}
}

float UHexGridGenerator::SphericalAngle(const FVector& center, const FVector& p1, const FVector& p2)
{
	FVector v1 = (p1 - center * FVector::DotProduct(p1, center)).GetSafeNormal();
	FVector v2 = (p2 - center * FVector::DotProduct(p2, center)).GetSafeNormal();

	float dot = FVector::DotProduct(v1, v2);
	return FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
}

void UHexGridGenerator::FindEdgeTriangles(const FTriangleMesh& mesh, int32 vA, int32 vB, TArray<int32>& outTriangleIndices)
{
	outTriangleIndices.Empty();

	const TArray<int32>* trianglesA = mesh.VertexToTriangleMap.Find(vA);
	const TArray<int32>* trianglesB = mesh.VertexToTriangleMap.Find(vB);

	if (!trianglesA || !trianglesB)
	{
		return;
	}

	// Find triangles that contain both vertices
	for (int32 triIdx : *trianglesA)
	{
		if (trianglesB->Contains(triIdx))
		{
			outTriangleIndices.Add(triIdx);
		}
	}
}
