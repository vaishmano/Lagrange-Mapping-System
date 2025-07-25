#pragma once

#include "math.hpp"

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

/// <summary>
/// Class that represents the reference grid.
/// </summary>
class Grid
{
public:
	/// <summary>
	/// Constructor.
	/// </summary>
	Grid()
	{
		int M = 11;
		int N = 11;
		double size = 1.5;
		vtkNew<vtkPoints> points;
		for (int m = 0; m < M; ++m) {
			double t = (-1 + 2 * m / (M - 1.0)) * size;
			points->InsertNextPoint(t, -size, -1E-2);
			points->InsertNextPoint(t, size, -1E-2);
		}
		for (int n = 0; n < N; ++n) {
			double t = (-1 + 2 * n / (N - 1.0)) * size;
			points->InsertNextPoint(-size, t, -1E-2);
			points->InsertNextPoint(size, t, -1E-2);
		}
		points->ComputeBounds();

		// create the index buffer
		vtkNew<vtkCellArray> cells;
		for (int n = 0; n < points->GetNumberOfPoints() / 2; ++n) {
			cells->InsertNextCell(2);
			cells->InsertCellPoint(2 * n);
			cells->InsertCellPoint(2 * n + 1);
		}

		vtkNew<vtkPolyData> polyData;
		polyData->SetPoints(points);
		polyData->SetLines(cells);
		
		vtkNew<vtkPolyDataMapper> mapper;
		mapper->SetInputData(polyData);

		mActor = vtkSmartPointer<vtkActor>::New();
		mActor->SetMapper(mapper);
		mActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
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
	Grid(const Grid&) = delete;				// Delete the copy-constructor.
	void operator=(const Grid&) = delete;	// Delete the assignment operator.

	vtkSmartPointer<vtkActor> mActor;		// Actor that stores the grid lines.
};