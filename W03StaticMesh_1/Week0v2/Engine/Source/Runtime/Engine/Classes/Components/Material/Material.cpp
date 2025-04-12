#include "Material.h"

#include "Components/Mesh/StaticMesh.h"
#include "UObject/Casts.h"

UMaterial* UMaterial::Duplicate()
{
    FDuplicateContext Context;
    return dynamic_cast<UMaterial*>( Duplicate(Context));
}

UObject* UMaterial::Duplicate(FDuplicateContext& Context)
{
    if (Context.DuplicateMap.Find(this))
    {
        return Context.DuplicateMap[this];
    }
    
    UMaterial* DuplicatedObject = reinterpret_cast<UMaterial*>(Super::Duplicate());
    memcpy(reinterpret_cast<char*>(DuplicatedObject) + sizeof(Super), reinterpret_cast<char*>(DuplicatedObject) + sizeof(Super), sizeof(UStaticMesh) - sizeof(Super));
    DuplicatedObject->materialInfo = materialInfo;

    return DuplicatedObject;
}
