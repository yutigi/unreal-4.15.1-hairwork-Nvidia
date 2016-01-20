// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
ShaderComplexityRendering.h: Declarations used for the shader complexity viewmode.
=============================================================================*/

#pragma once

// Used to know whether QuadOverdraw should be used at runtime.
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
FORCEINLINE bool RuntimeAllowDebugViewModeShader(ERHIFeatureLevel::Type InFeatureLevel)
{
	return AllowDebugViewModeShader(GetFeatureLevelShaderPlatform(InFeatureLevel));
}
#else
FORCEINLINE bool RuntimeAllowDebugViewModeShader(ERHIFeatureLevel::Type InFeatureLevel)
{
	return false;
}
#endif

/**
 * Vertex shader for quad overdraw. Required because overdraw shaders need to have SV_Position as first PS interpolant.
 */
class FDebugViewModeVS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FDebugViewModeVS,MeshMaterial);
protected:

	FDebugViewModeVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer) : FMeshMaterialShader(Initializer) {}
	FDebugViewModeVS() {}

public:

	static bool ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return AllowDebugViewModeShader(Platform) && (Material->IsSpecialEngineMaterial() || Material->HasVertexPositionOffsetConnected() || Material->GetTessellationMode() != MTM_NoTessellation);
	}

	void SetParameters(FRHICommandList& RHICmdList, const FMaterialRenderProxy* MaterialRenderProxy,const FMaterial& Material,const FSceneView& View)
	{
		FMeshMaterialShader::SetParameters(RHICmdList, GetVertexShader(),MaterialRenderProxy,Material,View,ESceneRenderTargetsMode::DontSet);
	}

	void SetMesh(FRHICommandList& RHICmdList, const FVertexFactory* VertexFactory,const FSceneView& View,const FPrimitiveSceneProxy* Proxy,const FMeshBatchElement& BatchElement,const FMeshDrawingRenderState& DrawRenderState)
	{
		FMeshMaterialShader::SetMesh(RHICmdList, GetVertexShader(),VertexFactory,View,Proxy,BatchElement,DrawRenderState);
	}
};

/**
 * Hull shader for quad overdraw. Required because overdraw shaders need to have SV_Position as first PS interpolant.
 */
class FDebugViewModeHS : public FBaseHS
{
	DECLARE_SHADER_TYPE(FDebugViewModeHS,MeshMaterial);
public:

	static bool ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return FBaseHS::ShouldCache(Platform, Material, VertexFactoryType) && FDebugViewModeVS::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FDebugViewModeHS(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FBaseHS(Initializer) {}
	FDebugViewModeHS() {}
};

/**
 * Domain shader for quad overdraw. Required because overdraw shaders need to have SV_Position as first PS interpolant.
 */
class FDebugViewModeDS : public FBaseDS
{
	DECLARE_SHADER_TYPE(FDebugViewModeDS,MeshMaterial);
public:

	static bool ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return FBaseDS::ShouldCache(Platform, Material, VertexFactoryType) && FDebugViewModeVS::ShouldCache(Platform,Material,VertexFactoryType);		
	}

	FDebugViewModeDS(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FBaseDS(Initializer) {}
	FDebugViewModeDS() {}
};

class FDebugViewModePS : public FGlobalShader
{
public:

	FDebugViewModePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}
	FDebugViewModePS() {}

	virtual void SetParameters(
		FRHICommandList& RHICmdList, 
		const FShader* OriginalVS, 
		const FShader* OriginalPS, 
		const FSceneView& View
		) = 0;

	virtual void SetMesh(
		FRHICommandList& RHICmdList, 
		const FVertexFactory* VertexFactory,
		const FSceneView& View,
		const FPrimitiveSceneProxy* Proxy,
		const FMeshBatchElement& BatchElement, 
		const FMeshDrawingRenderState& DrawRenderState
		) = 0;

	// Used for custom rendering like decals.
	virtual void SetMesh(FRHICommandList& RHICmdList, const FSceneView& View) = 0;
};

struct FDebugViewMode
{
	static FDebugViewModePS* GetPixelShader(TShaderMap<FGlobalShaderType>* ShaderMap, EDebugViewShaderMode DebugViewShaderMode);

	static void GetDebugMaterial(const FMaterialRenderProxy** MaterialRenderProxy, const FMaterial** Material, ERHIFeatureLevel::Type FeatureLevel);

	static void SetParametersVSHSDS(
		FRHICommandList& RHICmdList, 
		const FMaterialRenderProxy* MaterialRenderProxy, 
		const FMaterial* Material, 
		const FSceneView& View,
		const FVertexFactory* VertexFactory,
		bool bHasHullAndDomainShader
		);

	static void SetMeshVSHSDS(
		FRHICommandList& RHICmdList, 
		const FVertexFactory* VertexFactory,
		const FSceneView& View,
		const FPrimitiveSceneProxy* Proxy,
		const FMeshBatchElement& BatchElement, 
		const FMeshDrawingRenderState& DrawRenderState,
		const FMaterial* Material, 
		bool bHasHullAndDomainShader
		);

	static void PatchBoundShaderState(
			FBoundShaderStateInput& BoundShaderStateInput,
			const FMaterial* Material,
			const FVertexFactory* VertexFactory,
			ERHIFeatureLevel::Type FeatureLevel, 
			EDebugViewShaderMode DebugViewShaderMode
			);

};