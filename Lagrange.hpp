#pragma once

#include "math.hpp"
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkRenderer.h>
#include "crtbp.hpp"

/// <summary>
/// Computes and visualizes the five Lagrange points for the CRTBP.
/// </summary>
class LagrangePoints {
public:
    /// <summary>
    /// Constructor: computes all five Lagrange points via Newton–Raphson.
    /// </summary>
    LagrangePoints() {
        // Initial guesses along the x-axis for L1, L2, L3 and equilateral for L4/L5
        std::vector<Vector2d> guesses = {
            {0.8  , 0.0},          // L1 approx
            {1.2 , 0.0},          // L2 approx
            {-1.0, 0.0},         // L3 approx
            {0.5, 0.8}, // L4
            {0.5 , -0.8}  // L5
        };

        for (auto& x0 : guesses) {
            lagrangePoints_.push_back(FindPoint(x0));
        }

        // Setup shared geometry: one sphere source and mapper
        sphereSource_ = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource_->SetRadius(0.02);

        mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper_->SetInputConnection(sphereSource_->GetOutputPort());

        // Create actors
        const char* names[5] = { "L1","L2","L3","L4","L5" };
        for (int i = 0; i < 5; ++i) {
            auto actor = vtkSmartPointer<vtkActor>::New();
            actor->SetMapper(mapper_);
            actor->SetPosition(lagrangePoints_[i].x(), lagrangePoints_[i].y(), 0.0);
            actor->GetProperty()->SetColor(0.8, 0.8, 0.8);         // light grey
            actor->GetProperty()->SetAmbient(0.3);
            actors_.push_back(actor);

            auto label = vtkSmartPointer<vtkBillboardTextActor3D>::New();
            label->SetInput(names[i]);
            label->SetPosition(lagrangePoints_[i].x(), lagrangePoints_[i].y(), 0.05);
            labels_.push_back(label);
        }
    }

    /// <summary>
    /// Adds Lagrange point actors (and labels) to the given renderer.
    /// </summary>
    void InitRenderer(vtkRenderer* renderer) {
        for (auto& a : actors_) {
            renderer->AddActor(a);
        }
        for (auto& l : labels_) {
            renderer->AddActor(l);
        }
    }

    /// <summary>
    /// Returns the computed Lagrange point positions.
    /// </summary>
    const std::vector<Vector2d>& GetPoints() const { return lagrangePoints_; }

private:
    std::vector<Vector2d> lagrangePoints_;
    vtkSmartPointer<vtkSphereSource> sphereSource_;
    vtkSmartPointer<vtkPolyDataMapper> mapper_;
    std::vector<vtkSmartPointer<vtkActor>> actors_;
    std::vector<vtkSmartPointer<vtkBillboardTextActor3D>> labels_;

    /// <summary>
    /// Newton–Raphson solver using CRTBP::PseudoPotentialGrad and Hessian.
    /// </summary>
    Vector2d FindPoint(const Vector2d& x0) {
        Vector2d x = x0;
        for (int iter = 0; iter < 10; ++iter) {
            // Compute gradient and Hessian
            Eigen::Vector2d v = CRTBP::PseudoPotentialGrad(x);
            Eigen::Matrix2d H = CRTBP::PseudoPotentialHessian(x);
            // Newton step: x_{n+1} = x_n - H^{-1} * v
            Vector2d dx = H.lu().solve(v);
            x = x - dx;
            if (dx.norm() < 1e-8) break;
        }
        return x;
    }
};