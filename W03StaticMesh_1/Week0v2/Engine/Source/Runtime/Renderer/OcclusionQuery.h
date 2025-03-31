#pragma once
class ID3D11Query;
class FOcclusionQuery
{
public:
    FOcclusionQuery();
    ~FOcclusionQuery();

    ID3D11Query* Get() const { return Query; }

private:
    ID3D11Query* Query = nullptr;
};

