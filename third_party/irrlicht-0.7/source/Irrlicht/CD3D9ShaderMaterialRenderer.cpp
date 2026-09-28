
#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECTX_9_

#include "CD3D9ShaderMaterialRenderer.h"
#include "IVideoDriver.h"
#include "os.h"


namespace irr
{
namespace video  
{

CD3D9ShaderMaterialRenderer::CD3D9ShaderMaterialRenderer(IDirect3DDevice9* d3ddev, video::IVideoDriver* driver, 
		s32& outMaterialTypeNr, const c8* vertexShaderProgram, const c8* pixelShaderProgram,
		IShaderConstantSetCallBack* callback, IMaterialRenderer* baseMaterial)
: pID3DDevice(d3ddev), Driver(driver), BaseMaterial(baseMaterial), CallBack(callback),
	VertexShader(0), PixelShader(0)
{
	if (BaseMaterial)
		BaseMaterial->grab();

	if (CallBack)
		CallBack->grab();

	outMaterialTypeNr = -1;

	// create vertex shader
	if (!createVertexShader(vertexShaderProgram))
		return;

	// create pixel shader
	if (!createPixelShader(pixelShaderProgram))
		return;

	// register myself as new material
	outMaterialTypeNr = driver->addMaterialRenderer(this);
}



//! Destructor
CD3D9ShaderMaterialRenderer::~CD3D9ShaderMaterialRenderer()
{
	if (CallBack)
		CallBack->drop();

	if (VertexShader)
		VertexShader->Release();

	if (PixelShader)
		PixelShader->Release();

	if (BaseMaterial)
		BaseMaterial->drop ();
}


void CD3D9ShaderMaterialRenderer::OnSetMaterial(video::SMaterial& material, const video::SMaterial& lastMaterial,
	bool resetAllRenderstates, video::IMaterialRendererServices* services) 
{
	if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
	{
		// call callback to set shader constants
		if (CallBack)
			CallBack->OnSetConstants(services);
		
		if (VertexShader)
		{
			// save old vertex shader
			pID3DDevice->GetVertexShader(&OldVertexShader);
			
			// set new vertex shader
			if (FAILED(pID3DDevice->SetVertexShader(VertexShader)))
				os::Printer::log("Could not set vertex shader.");
		}

		// set new pixel shader
		if (PixelShader)
		{
			if (FAILED(pID3DDevice->SetPixelShader(PixelShader)))
				os::Printer::log("Could not set pixel shader.");
		}

		if (BaseMaterial)
			BaseMaterial->OnSetMaterial(material, material, true, services);
	}

	services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}

void CD3D9ShaderMaterialRenderer::OnUnsetMaterial() 
{
	if (VertexShader)
		pID3DDevice->SetVertexShader(OldVertexShader);

	if (PixelShader)
		pID3DDevice->SetPixelShader(0);

	if (BaseMaterial)
		BaseMaterial->OnUnsetMaterial();
}

	
//! Returns if the material is transparent. The scene managment needs to know this
//! for being able to sort the materials by opaque and transparent.
bool CD3D9ShaderMaterialRenderer::isTransparent() 
{
	return BaseMaterial ? BaseMaterial->isTransparent() : false; 
}

bool CD3D9ShaderMaterialRenderer::createPixelShader(const c8* pxsh)
{
	if (!pxsh)
		return true;

	// compile shader

	LPD3DXBUFFER code = 0;
	LPD3DXBUFFER errors = 0;
	D3DXAssembleShader(pxsh, strlen(pxsh), 0, 0, 0, &code, &errors);

	if (errors)
	{
		// print out compilation errors.
		os::Printer::log("Pixel shader compilation failed:");
		os::Printer::log((c8*)errors->GetBufferPointer());

		errors->Release();
		return false;
	}

	if (FAILED(pID3DDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &PixelShader)))
	{
		os::Printer::log("Could not create pixel shader.");
		code->Release();
		return false;
	}

	code->Release();
	return true;
}



bool CD3D9ShaderMaterialRenderer::createVertexShader(const char* vtxsh)
{
	if (!vtxsh)
		return true;

	// compile shader

	LPD3DXBUFFER code = 0;
	LPD3DXBUFFER errors = 0;
	D3DXAssembleShader(vtxsh, strlen(vtxsh), 0, 0, 0, &code, &errors);

	if (errors)
	{
		// print out compilation errors.
		os::Printer::log("Vertex shader compilation failed:");
		os::Printer::log((c8*)errors->GetBufferPointer());

		errors->Release();
		return false;
	}

	if (FAILED(pID3DDevice->CreateVertexShader((DWORD*)code->GetBufferPointer(), &VertexShader)))
	{
		os::Printer::log("Could not create vertex shader.");
		code->Release();
		return false;
	}

	code->Release();
	return true;
}



} // end namespace video
} // end namespace irr

#endif

