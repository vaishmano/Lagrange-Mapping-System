#pragma once

#include "crtbp.hpp"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
/// <summary>
/// Class that represents the third body.
/// </summary>
class Tracer
{
private:
	static constexpr double MaxRadius = 0.01;				// max radius of the tube
	static constexpr int NumIntegrationSteps = 1000;		// number of integration steps
	static constexpr double IntegrationStepSize = 0.005;		// integration step size

	// Step 1: vtkPolyData to store the trajectory
	vtkSmartPointer<vtkPolyData> trajectory;

	// Step 2: Mapper
	vtkSmartPointer<vtkPolyDataMapper> trajectoryMapper;

	// Step 3: Actor
	vtkSmartPointer<vtkActor> trajectoryActor;

	vtkSmartPointer<vtkTubeFilter> tubeFilter;
	vtkSmartPointer<vtkFloatArray> radiusArray;

public:
	/// <summary>
	/// Constructor.
	/// </summary>
	Tracer()
	{
		// Step 1: Create trajectory BEFORE using it
		trajectory = vtkSmartPointer<vtkPolyData>::New();

		// Step 2: Create the tube filter and connect it
		tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
		tubeFilter->SetInputData(trajectory);
		tubeFilter->SetNumberOfSides(24);   // Smooth tube
		tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
		tubeFilter->SetRadius(MaxRadius);

		// Step 3: Mapper (connect to tube output)
		trajectoryMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		trajectoryMapper->SetInputConnection(tubeFilter->GetOutputPort());
		trajectoryMapper->ScalarVisibilityOff();

		// Step 4: Actor
		trajectoryActor = vtkSmartPointer<vtkActor>::New();
		trajectoryActor->SetMapper(trajectoryMapper);

		// Bronze/copper color
		auto prop = trajectoryActor->GetProperty();
		prop->SetColor(0.9, 0.3, 0.3); // (R,G,B) in range 0–1
		prop->SetOpacity(0.95);         // Slightly less transparent for a solid look
		prop->SetSpecular(0.0);         // Add some specular reflection for “metal”
		prop->SetSpecularPower(1.0);   // Controls shininess

		prop->LightingOn();             // Optional: turn ON lighting for metallic shine

		// Step 6: Radius array (create, but don't set size yet)
		radiusArray = vtkSmartPointer<vtkFloatArray>::New();
		radiusArray->SetName("TubeRadius");

		Pick(Vector3d(1.019, -0.008, 0.0));

	}

	/// <summary>
	/// Updates the properties of the tail geometry.
	/// </summary>
	/// <param name="dt">Time passed since the last Update in milliseconds.</param>
	/// <param name="t">Total time passed since start of the application in milliseconds.</param>
	void Update(double dt, double t)
	{
		if (!radiusArray) return;

		// Rotate the radius array forward by one (pulse moves ahead)
		float last = radiusArray->GetValue(radiusArray->GetNumberOfTuples() - 1);
		for (vtkIdType i = radiusArray->GetNumberOfTuples() - 1; i > 0; --i) {
			radiusArray->SetValue(i, radiusArray->GetValue(i - 1));
		}
		radiusArray->SetValue(0, last);

		// Apply updated radii
		trajectory->GetPointData()->SetScalars(radiusArray);
		trajectory->Modified();
		tubeFilter->Update();
	}

	/// <summary>
	/// Calculates a trajectory from the picked point.
	/// </summary>
	/// <param name="pnt">3D world coordinate that was picked.</param>
	void Pick(const Vector3d& pnt)
	{
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
		Vector2d pos(pnt.x(), pnt.y());
		//double jacobi_C = 3.1396645; // This is your yellow isoline
		double jacobi_C = 3.139855;
		double U = CRTBP::PseudoPotential(pos);
		double vmag = sqrt(std::max(2 * U - jacobi_C, 0.0));
		Vector2d earth = CRTBP::Earth();
		Vector2d rel = pos - earth;
		Vector2d vel(-rel.y(), rel.x());
		vel.normalize();
		double angle = -0.008; // try between 0.005 and 0.015 radians
		double cs = cos(angle), sn = sin(angle);
		Vector2d vel2(vel.x() * cs - vel.y() * sn, vel.x() * sn + vel.y() * cs);
		vel2.normalize();
		vel2 *= vmag;



		Vector4d state(pos.x(), pos.y(), vel2.x(), vel2.y());

		double t = 0.0;
		points->InsertNextPoint(state[0], state[1], 0.0);
		vtkIdType prevId = 0;

		for (int i = 1; i < NumIntegrationSteps; ++i)
		{
			Vector4d k1 = CRTBP::Direction(state);
			Vector4d k2 = CRTBP::Direction(state + (IntegrationStepSize / 2.0) * k1);
			Vector4d k3 = CRTBP::Direction(state + (IntegrationStepSize / 2.0) * k2);
			Vector4d k4 = CRTBP::Direction(state + IntegrationStepSize * k3);

			Vector4d nextState = state + (IntegrationStepSize / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);

			state = nextState;
			t += IntegrationStepSize;

			vtkIdType id = points->InsertNextPoint(state[0], state[1], 0.0);
			lines->InsertNextCell(2);
			lines->InsertCellPoint(prevId);
			lines->InsertCellPoint(id);
			prevId = id;
		}

		// Update vtkPolyData
		trajectory->SetPoints(points);
		trajectory->SetLines(lines);
		radiusArray->SetNumberOfValues(points->GetNumberOfPoints());
		float maxRadius = MaxRadius;  // e.g. 0.01
		float minRadius = 0.002f; // or 0.002f for even thinner start
		for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
		{

			float scalar = minRadius + (maxRadius - minRadius) * (static_cast<float>(i) / (points->GetNumberOfPoints() - 1));
			radiusArray->SetValue(i, scalar);

		}
		// Attach the array to points:
		trajectory->GetPointData()->SetScalars(radiusArray);
		trajectory->Modified();
		tubeFilter->Update();

	}

	/// <summary>
	/// Adds the actors to the renderer.
	/// </summary>
	/// <param name="renderer">Renderer to add the actors to.</param>
	void InitRenderer(vtkSmartPointer<vtkRenderer> renderer)
	{
		renderer->AddActor(trajectoryActor);
	}

private:

	Tracer(const Tracer&) = delete;				// Delete the copy-constructor.
	void operator=(const Tracer&) = delete;		// Delete the assignment operator.
};