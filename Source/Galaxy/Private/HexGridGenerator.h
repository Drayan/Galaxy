// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "HexGridGenerator.generated.h"

/// <summary>
/// Temporary structure for mesh generation (triangle mesh before converting to hex dual)
/// </summary>
struct FTriangleMesh
{
	/// <summary>
	/// Vertex positions on unit sphere
	/// </summary>
	TArray<FVector> Vertices;

	/// <summary>
	/// Triangle faces (indices into Vertices array, groups of 3)
	/// </summary>
	TArray<int32> Indices;

	/// <summary>
	/// Adjacency : for each vertex, list of triangle indices that include this vertex
	/// </summary>
	TMap<int32, TArray<int32>> VertexToTriangleMap;

	/// <summary>
	/// Adjacency : for each triangle, list of neighboring triangle indices
	/// </summary>
	TArray<TArray<int32>> TriangleNeighbors;

	void Clear();
	int32 GetTriangleCount() const { return Indices.Num() / 3; }
	void GetTriangle(int32 triIdx, int32& outV0, int32& outV1, int32& outV2) const;
	FVector GetTriangleCenter(int32 triIdx) const;
};

/// <summary>
/// Static utility class for generating spherical hexagonal grids
/// </summary>
UCLASS()
class GALAXY_API UHexGridGenerator : public UObject
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Generate a complete hex grid at the specified subdivision level
	/// </summary>
	/// <param name="level">Subdivision level (3-8) recommended, higher = more cells, but could increase generation time</param>
	/// <param name="OutErrors">Array to receive any error messages</param>
	/// <returns>Generated hex grid asset, or nullptr on failure</returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Grid Generation")
	static UHexGridAsset* GenerateHexGrid(int32 level, TArray<FString>& OutErrors);

	/// <summary>
	/// Create and save a hex grid asset to the content browser
	/// </summary>
	/// <param name="level">Subdivision level (3-8) recommended</param>
	/// <param name="assetName">Name of the asset to create (e.g. "HexGrid_L6")</param>
	/// <param name="assetPath">Path to save the asset (e.g. "/Game/HexGrids/")</param>
	/// <param name="OutErrors">Array to receive any error messages</param>
	/// <returns>Created hex grid asset, or nullptr on failure</returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Grid Generation")
	static UHexGridAsset* CreateHexGridAsset(int32 level, const FString& assetName, const FString& assetPath, TArray<FString>& OutErrors);

private:
	// === Generation Pipeline ===

	/// <summary>
	/// Step 1 : Create base icosahedron (12 vertices, 20 triangular faces)
	/// </summary>
	/// <param name="outMesh">The created mesh</param>
	static void CreateIcosahedron(FTriangleMesh& outMesh);
	
	/// <summary>
	/// Step 2 : Subdivide each triangle face into smaller triangles, N times.
	/// </summary>
	/// <param name="mesh">Mesh to subdivide</param>
	/// <param name="subdivisions">Number of subdivision to apply</param>
	static void SubdivideMesh(FTriangleMesh& mesh, int32 subdivisions);

	/// <summary>
	/// Step 3 : Build adjacency data for the triangle mesh (vertex->triangles, triangle->neighbor)
	/// </summary>
	/// <param name="mesh">Mesh to build adjacency data</param>
	static void BuildAdjacencyData(FTriangleMesh& mesh);

	/// <summary>
	/// Step 4 : Convert triangle mesh to hexagonal dual grid
	/// Each vertex in the triangle mesh becomes a hex cell in the hex grid
	/// Each triangle face in the triangle mesh becomes a vertex in the hex grid
	/// </summary>
	/// <param name="triMesh">The input mesh</param>
	/// <param name="outGrid">The output hex grid</param>
	static void ConvertToHexDual(const FTriangleMesh& triMesh, UHexGridAsset* outGrid);

	/// <summary>
	/// Step 5 : Build neighbor relationships between hex cells
	/// </summary>
	/// <param name="grid">The output grid to build neighbors for</param>
	static void BuildCellNeighbors(UHexGridAsset* grid);
	
	/// <summary>
	/// Step 6 : Order the vertices of each hex cell in consistent winding order (counter-clockwise)
	/// </summary>
	/// <param name="grid">The output grid to order cell vertices for</param>
	static void OrderCellVertices(UHexGridAsset* grid);

	/// <summary>
	/// Step 7 : Assign each hex cell to one of the 20 icosahedron faces
	/// </summary>
	/// <param name="grid">The output grid to assign icosahedron faces for</param>
	static void AssignIcosahedronFaces(UHexGridAsset* grid);

	// === Heper functions ===

	/// <summary>
	/// Get or add a vertex to the mesh, merging with existing vertex if within threshold
	/// </summary>
	/// <param name="mesh">Mesh to add to</param>
	/// <param name="position">Position on unit sphere (will be normalized)</param>
	/// <param name="mergeThreshold">Distance threshold for merging vertices</param>
	/// <returns></returns>
	static int32 GetOrAddVertex(FTriangleMesh& mesh, FVector position, float mergeThreshold = 0.0001f);

	/// <summary>
	/// Subdivide a triangle into 4 smaller triangles, adding new vertices to the mesh
	///
	///      V0
	///      /\
	///     /  \
	///   M01--M02
	///    /\  /\
	///   /  \/  \
	/// V1---M12--V2
	/// </summary>
	/// <param name="mesh">The original mesh</param>
	/// <param name="v0">Vertex 0</param>
	/// <param name="v1">Vertex 1</param>
	/// <param name="v2">Vertex 2</param>
	/// <param name="outNewVertices">New vertices created from the subdivision</param>
	static void SubdivideTriangle(FTriangleMesh& mesh, int32 v0, int32 v1, int32 v2, TArray<int32>& outNewVertices);

	/// <summary>
	/// Find the edge between two vertices in the triangle list, and return the triangle indices that include this edge
	/// </summary>
	/// <param name="mesh">The original mesh</param>
	/// <param name="vA">Vertex A</param>
	/// <param name="vB">Vertex B</param>
	/// <param name="outTriangleIndices">Triangle indices that include the edge</param>
	static void FindEdgeTriangles(const FTriangleMesh& mesh, int32 vA, int32 vB, TArray<int32>& outTriangleIndices);

	/// <summary>
	/// Check if two positions are equal within a certain tolerance
	/// </summary>
	/// <param name="posA">First position</param>
	/// <param name="posB">Second position</param>
	/// <param name="tolerance">Tolerance for equality check</param>
	/// <returns>True if positions are equal within tolerance, false otherwise</returns>
	static bool PositionsEqual(const FVector& posA, const FVector& posB, float tolerance = 0.0001f)
	{
		return FVector::DistSquared(posA, posB) <= tolerance * tolerance;
	}

	/// <summary>
	/// Calculate the spherical angle (in radians) at 'center' between points p1 and p2
	/// </summary>
	/// <param name="center">Center point</param>
	/// <param name="p1">First point</param>
	/// <param name="p2">Second point</param>
	/// <returns>The spherical angle (in radians) at 'center' between points p1 and p2</returns>
	static float SphericalAngle(const FVector& center, const FVector& p1, const FVector& p2);

	/// <summary>
	/// Order a list of vertices in counter-clockwise order around a center point
	/// </summary>
	/// <param name="center">Center point</param>
	/// <param name="vertices">List of vertices to order</param>
	static void OrderVerticesCounterClockwise(const FVector& center, TArray<FVector>& vertices);
};
