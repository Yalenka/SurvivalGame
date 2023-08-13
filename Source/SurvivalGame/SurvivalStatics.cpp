// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "SurvivalStatics.h"

UClass* USurvivalStatics::GetObjectParentClass(UObject* Object)
{
	return Object->GetClass()->GetSuperClass();
}
