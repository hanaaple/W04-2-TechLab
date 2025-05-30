#pragma once
#include <memory>
#include "Texture.h"
#include "Container/Map.h"

class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    HRESULT LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);
    HRESULT LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);

    std::shared_ptr<FTexture> GetTexture(const FWString& FilePath) const;

    std::shared_ptr<FTexture> GetTextureByName(const FWString& Name) const;

    const TMap<FWString, std::shared_ptr<FTexture>> GetAllTextures() const { return textureMap; }
private:
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
    TMap<FWString, std::shared_ptr<FTexture>> textureMapByName;
};