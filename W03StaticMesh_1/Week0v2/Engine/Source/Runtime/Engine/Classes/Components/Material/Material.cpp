#include "Material.h"

#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
UObject* UMaterial::Duplicate()
{
    UMaterial* dup = Cast<UMaterial>(FObjectFactory::DuplicateObject(this, this->GetClass()));
    dup->materialInfo = materialInfo;

    return dup;
}
