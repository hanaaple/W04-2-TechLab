#include "Material.h"

#include "UObject/Casts.h"

void UMaterial::CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap)
{
    Super::CopyPropertiesFrom(Source, DupMap);
    UMaterial* SourceUMaterial = Cast<UMaterial>(Source);
    if (SourceUMaterial)
    {
        materialInfo = SourceUMaterial->materialInfo;
    }
}

void UMaterial::CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& DupMap)
{
    UObject::CopyPropertiesTo(Dest, DupMap);

    UMaterial* DestUMaterial = Cast<UMaterial>(Dest);
    if (DestUMaterial)
    {
        DestUMaterial->materialInfo = materialInfo;
    }
}
