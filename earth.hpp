#pragma once

#include "crtbp.hpp"

#include <vtkSphereSource.h>
#include <vtkTexturedSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <string>
#include <vtkJPEGReader.h>
#include <vtkNew.h>
#include <vtkTexture.h>
#include <vtkImageData.h>

/// <summary>
/// Class that represents the Earth.
/// </summary>
class Earth
{
public:
	/// <summary>
	/// Constructor.
	/// </summary>
	Earth()
	{


		// Load the image using jpeg reader, I referred VTK documentation for this.
		vtkNew<vtkJPEGReader> jpegReader;
		jpegReader->SetFileName("./../images/earth.jpg");
		jpegReader->Update();

		vtkNew<vtkTexture> texture;
		texture->SetInputData(jpegReader->GetOutput());
		texture->UseSRGBColorSpaceOn();
		// Create a sphere with texture coordinates
		const double sphere_radius = 0.05;
		vtkNew<vtkTexturedSphereSource> sphereSource;
		sphereSource->SetRadius(sphere_radius);
		sphereSource->SetPhiResolution(100);
		sphereSource->SetThetaResolution(100);
		vtkNew<vtkPolyDataMapper> mapper;
		mapper->SetInputConnection(sphereSource->GetOutputPort());

		mActor = vtkSmartPointer<vtkActor>::New();
		mActor->SetMapper(mapper);
		mActor->SetPosition(CRTBP::Earth().x(), CRTBP::Earth().y(), 0);


		// — PBR setup begins here —
		auto prop = mActor->GetProperty();
		prop->SetInterpolationToPBR();
		// 1) Set a flat base color for the Sun (e.g. warm yellow)
		prop->SetColor(0.2, 0.4, 0.8);   // RGB in [0,1]
		prop->SetBaseColorTexture(texture);

		// 2) Tune metallic & roughness
		prop->SetMetallic(0.3);          // non-metal
		prop->SetRoughness(0.5);
	}

	/// <summary>
	/// Updates the properties of the Earth.
	/// </summary>
	/// <param name="dt">Time passed since the last Update in milliseconds.</param>
	/// <param name="t">Total time passed since start of the application in milliseconds.</param>
	void Update(double dt, double t)
	{
		// rotate actor
		double angle_dt = dt * 0.1;							// not a realistic rotation speed!
		mActor->RotateWXYZ(angle_dt, 0, 0.398749, 0.91706);	// 23.5� titled rotation axis
	}

	/// <summary>
	/// Adds the actors to the renderer.
	/// </summary>
	/// <param name="renderer">Renderer to add the actors to.</param>
	void InitRenderer(vtkSmartPointer<vtkRenderer> renderer)
	{
		renderer->AddActor(mActor);
	}

private:
	Earth(const Earth&) = delete;			// Delete the copy-constructor.
	void operator=(const Earth&) = delete;	// Delete the assignment operator.

	vtkSmartPointer<vtkActor> mActor;		// Actor that represents the scene geometry.
};