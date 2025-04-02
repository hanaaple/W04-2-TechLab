#pragma once
#include "Container/String.h"
#include "D3D11RHI/GraphicDevice.h"
struct FTexture
{
    FTexture(ID3D11ShaderResourceView* SRV, ID3D11Texture2D* Texture2D, ID3D11SamplerState* Sampler, uint32 _width, uint32 _height, FWString _name, FWString _fileName)
        : TextureSRV(SRV), Texture(Texture2D), SamplerState(Sampler), width(_width), height(_height), DisplayName(_name), FileName(_fileName)
    {}
    ~FTexture()
    {
		
    }
    void Release() {
        if (TextureSRV) { TextureSRV->Release(); TextureSRV = nullptr; }
        if (Texture) { Texture->Release(); Texture = nullptr; }
        if (SamplerState) { SamplerState->Release(); SamplerState = nullptr; }
    }

    FWString DisplayName;
    FWString FileName;
    
    ID3D11ShaderResourceView* TextureSRV = nullptr;
    ID3D11Texture2D* Texture = nullptr;
    ID3D11SamplerState* SamplerState = nullptr;
    uint32 width;
    uint32 height;
};
