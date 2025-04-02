#include "Material.h"

#include "UObject/Casts.h"

void UMaterial::CopyPropertiesFrom(UObject* Source)
{
    Super::CopyPropertiesFrom(Source);
    UMaterial* SourceUMaterial = Cast<UMaterial>(Source);
    if (SourceUMaterial)
    {
        materialInfo = SourceUMaterial->materialInfo;
    }
}
