#pragma once

#include "crtbp.hpp"

#include <vtkSphereSource.h>
#include <vtkTexturedSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkJPEGReader.h>
#include <vtkNew.h>
#include <vtkTexture.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>

/// <summary>
/// Class that represents the sun.
/// </summary>
class Sun
{
public:
	/// <summary>
	/// Constructor.
	/// </summary>
	Sun()

	{

		vtkNew<vtkImageData> volumeObject;
		// Set dimensions — this creates a 64×64×64 voxel grid
		const int dim = 64;
		volumeObject->SetDimensions(dim, dim, dim);

		// Set spacing — how far apart the voxels are in world coordinates
		// Smaller spacing = more detail but also more memory cost
		volumeObject->SetSpacing(0.005, 0.005, 0.005);

		// Center it around (0,0,0) by shifting the origin accordingly
		// Since origin is at the corner of the volume, we shift it so (0,0,0) is in the middle
		volumeObject->SetOrigin(-dim / 2 * 0.005, -dim / 2 * 0.005, -dim / 2 * 0.005);

		// Allocate memory for scalar values (1 component per voxel, float type)
		volumeObject->AllocateScalars(VTK_FLOAT, 1);

		this->SampleGlowWithSmoothstep(volumeObject);

		// 1) Color: white→bright yellow→transparent
		vtkNew<vtkColorTransferFunction> colorTF;
		colorTF->AddRGBPoint(1.0, 1.0, 1.0, 1.0); // core: white
		colorTF->AddRGBPoint(0.8, 1.0, 1.0, 0.6); // ring: bright yellow
		colorTF->AddRGBPoint(0.5, 1.0, 1.0, 0.6); // outer: orange
		colorTF->AddRGBPoint(0.0, 1.0, 0.6, 0.5); // beyond: transparent

		// 2) Opacity: 0 at center → peak at ring → fade out
		vtkNew<vtkPiecewiseFunction> opacityTF;
		opacityTF->AddPoint(1.0, 0.9); // center: invisible
		opacityTF->AddPoint(0.9, 0.8); // just outside sphere: strong glow
		opacityTF->AddPoint(0.5, 0.7); // mid‐halo: medium glow
		opacityTF->AddPoint(0.4, 0.5);
		opacityTF->AddPoint(0.3, 0.1); // outer‐halo: faint glow
		opacityTF->AddPoint(0.0, 0.0); // beyond: nothing

		// 3) Hook into volume property
		vtkNew<vtkVolumeProperty> volumeProp;
		volumeProp->SetColor(colorTF);
		volumeProp->SetScalarOpacity(opacityTF);

		// 4) Create the GPU volume mapper and feed it the scalar volume
		vtkNew<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
		volumeMapper->SetInputData(volumeObject);

		// 5) set the vtkVolume (the “actor” for volumes)
		volumeActor->SetMapper(volumeMapper);
		volumeActor->SetProperty(volumeProp);

		// 6) Position the volume at the Sun’s location
		double sunX = CRTBP::Sun().x();
		double sunY = CRTBP::Sun().y();
		volumeActor->SetPosition(sunX, sunY, 0.0);

		// Load the image using jpeg reader, I referred VTK documentation for this.
		vtkNew<vtkJPEGReader> jpegReader;
		jpegReader->SetFileName("./../images/sun.jpg");
		jpegReader->Update();

		vtkNew<vtkTexture> texture;

		texture->SetInputData(jpegReader->GetOutput());
		texture->UseSRGBColorSpaceOn();
		// Create a sphere with texture coordinates
		const double sphere_radius = 0.1;
		vtkNew<vtkTexturedSphereSource> sphereSource;
		sphereSource->SetRadius(sphere_radius);
		sphereSource->SetPhiResolution(100);
		sphereSource->SetThetaResolution(100);
		vtkNew<vtkPolyDataMapper> polyMapper;

		polyMapper->SetInputConnection(sphereSource->GetOutputPort());

		// create actor
		mActor = vtkSmartPointer<vtkActor>::New();
		mActor->SetMapper(polyMapper);
		mActor->SetPosition(CRTBP::Sun().x(), CRTBP::Sun().y(), 0);

		// — PBR setup begins here —
		auto prop = mActor->GetProperty();
		prop->SetInterpolationToPBR();
		// 1) Set a flat base color for the Sun (e.g. warm yellow)
		prop->SetColor(1.0, 0.9, 0.5); // RGB in [0,1]
		prop->SetBaseColorTexture(texture);

		// 2) Tune metallic & roughness
		prop->SetMetallic(0.1); // non-metal
		prop->SetRoughness(1.0);
		prop->SetEmissiveTexture(texture);
		prop->SetEmissiveFactor(5.0, 5.0, 5.0);
	}

	/// <summary>
	/// Adds the actors to the renderer.
	/// </summary>
	/// <param name="renderer">Renderer to add the actors to.</param>
	void InitRenderer(vtkSmartPointer<vtkRenderer> renderer)
	{
		renderer->AddActor(mActor);
		renderer->AddVolume(volumeActor);
	}

private:
	Sun(const Sun&) = delete; // Delete the copy-constructor.
	void operator=(const Sun&) = delete;
	void SampleGlowWithSmoothstep(vtkImageData* imageData)
	{
		const double radius = 0.25; // glow radius in world units

		int* dims = imageData->GetDimensions();
		double* origin = imageData->GetOrigin();
		double* spacingVals = imageData->GetSpacing();

		for (int z = 0; z < dims[2]; ++z)
		{
			for (int y = 0; y < dims[1]; ++y)
			{
				for (int x = 0; x < dims[0]; ++x)
				{
					// Convert voxel index to world coordinates
					double px = origin[0] + x * spacingVals[0];
					double py = origin[1] + y * spacingVals[1];
					double pz = origin[2] + z * spacingVals[2];

					double r = std::sqrt(px * px + py * py + pz * pz) / radius;

					float value;
					if (r >= 1.0)
						value = 0.0f;
					else
					{
						float t = static_cast<float>(r);
						float s = 3 * t * t - 2 * t * t * t; // smoothstep
						value = 1.0f - s;					 // invert: 1 at center, 0 at edge
					}

					float* pixel = static_cast<float*>(imageData->GetScalarPointer(x, y, z));
					*pixel = value;
				}
			}
		} // Delete the assignment operator.
	}

	vtkSmartPointer<vtkActor> mActor; // Actor that represents the sun geometry.
	vtkNew<vtkVolume> volumeActor;
};