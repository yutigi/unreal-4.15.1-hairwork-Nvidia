// @third party code - BEGIN HairWorks
#pragma once

#include "HairWorksAsset.generated.h"

namespace nvidia{namespace HairWorks{
	enum AssetId;
}}
class UHairWorksMaterial;

UCLASS(BlueprintType)
class ENGINE_API UHairWorksAsset : public UObject
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITORONLY_DATA
	/** Importing data and options used for this HairWorks asset */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bGroom = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bMaterials = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bConstraints = true;

	UPROPERTY(VisibleAnywhere, Category = ImportSettings)
	bool bTextures = false;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bCollisions = true;
	
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = Hair)
	UHairWorksMaterial* HairMaterial;

	~UHairWorksAsset();

	// Begin UObject interface.
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	// End UObject interface.

	UPROPERTY()
	TArray<uint8> AssetData;

	UPROPERTY()
	TArray<FName> BoneNames;

	nvidia::HairWorks::AssetId AssetId;

	uint32 PinsUpdateFrameNumber = -1;	// Update pin data once per frame
};
// @third party code - END HairWorks