#include "Renderer/Renderer.h"
#include "UParticleSubUVComp.h"
#include "UnrealEd/EditorViewportClient.h"
#include "World.h"

UParticleSubUVComp::UParticleSubUVComp()
{
    SetType(StaticClass()->GetName());
    bIsLoop = true;
}

UParticleSubUVComp::~UParticleSubUVComp()
{
	if (vertexSubUVBuffer)
	{
		vertexSubUVBuffer->Release();
		vertexSubUVBuffer = nullptr;
	}
}

void UParticleSubUVComp::InitializeComponent()
{
	Super::InitializeComponent();
	FEditorEngine::renderer.UpdateSubUVConstant(0, 0);
	FEditorEngine::renderer.PrepareSubUVConstant();
}

void UParticleSubUVComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (!IsActive()) return;

	uint32 CellWidth = GetTexture()->width / CellsPerColumn;
	uint32 CellHeight = GetTexture()->height / CellsPerColumn;


	second += DeltaTime;
	if (second >= 75)
	{
		indexU++;
		second = 0;
	}
	if (indexU >= CellsPerColumn)
	{
		indexU = 0;
		indexV++;
	}
	if (indexV >= CellsPerRow)
	{
		indexU = 0;
		indexV = 0;

	    // TODO: 파티클 제거는 따로 안하고, Actor에 LifeTime을 설정하든가, 파티클의 Activate 설정을 추가하던가 하기로
	    if (!bIsLoop)
	    {
            Deactivate();
	    }
	    // DestroyComponent();
		// GetWorld()->ThrowAwayObj(this);
		// GetWorld()->SetPickingObj(nullptr);
	}


	float normalWidthOffset = float(CellWidth) / float(GetTexture()->width);
	float normalHeightOffset = float(CellHeight) / float(GetTexture()->height);

	finalIndexU = float(indexU) * normalWidthOffset;
	finalIndexV = float(indexV) * normalHeightOffset;
}

void UParticleSubUVComp::SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn)
{
	CellsPerRow = _cellsPerRow;
	CellsPerColumn = _cellsPerColumn;

	CreateSubUVVertexBuffer();
}

// UObject* UParticleSubUVComp::Duplicate()
// {
//     UParticleSubUVComp* dup = Cast<UParticleSubUVComp>(FObjectFactory::DuplicateObject(this, this->GetClass()));
//     dup->vertexSubUVBuffer = vertexSubUVBuffer;
//     dup->numTextVertices = numTextVertices;
//     dup->bIsLoop = bIsLoop;
//     dup->indexU = indexU;
//     dup->indexV = indexV;
//     dup->second = second;
//     dup->CellsPerColumn = CellsPerColumn;
//     dup->CellsPerRow = CellsPerRow;
//
//     return dup;
// }

void UParticleSubUVComp::UpdateVertexBuffer(const TArray<FVertexTexture>& vertices)
{
	/*
	ID3D11DeviceContext* context = FEngineLoop::graphicDevice.DeviceContext;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	context->Map(vertexTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, vertices.GetData(), vertices.Num() * sizeof(FVertexTexture));
	context->Unmap(vertexTextureBuffer, 0);
	*/

}

void UParticleSubUVComp::CreateSubUVVertexBuffer()
{

	uint32 CellWidth = GetTexture()->width/CellsPerColumn;
	uint32 CellHeight = GetTexture()->height/ CellsPerColumn;
	float normalWidthOffset = float(CellWidth) / float(GetTexture()->width);
	float normalHeightOffset = float(CellHeight) / float(GetTexture()->height);

	TArray<FVertexTexture> vertices =
	{
		{-1.0f,1.0f,0.0f,0,0},
		{ 1.0f,1.0f,0.0f,1,0},
		{-1.0f,-1.0f,0.0f,0,1},
		{ 1.0f,-1.0f,0.0f,1,1}
	};
	vertices[1].u = normalWidthOffset;
	vertices[2].v = normalHeightOffset;
	vertices[3].u = normalWidthOffset;
	vertices[3].v = normalHeightOffset;

	vertexSubUVBuffer = FEditorEngine::renderer.CreateVertexBuffer(vertices.GetData(), static_cast<UINT>(vertices.Num() * sizeof(FVertexTexture)));
	numTextVertices = static_cast<UINT>(vertices.Num());
}
