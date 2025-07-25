#pragma once
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkCommand.h>
#include <vtkProperty.h>
#include <vtkOutlineFilter.h>

#include "crtbp.hpp" // Your CRTBP class header



class JacobiConstant
{
public:
    JacobiConstant()
    {

        CreateGrid();
        SampleField();
        CreateContour();
    }

    vtkSmartPointer<vtkImageData> GetImageData() const { return m_imageData; }

    void InitRenderer(vtkRenderer* renderer)
    {
        if (renderer && m_contourActor && m_outlineActor)
        {
            renderer->AddActor(m_contourActor);
            renderer->AddActor(m_outlineActor);
        }
    }

    void InitUI(vtkRenderWindowInteractor* interactor)
    {
        if (!interactor) return;

        // --- Slider setup ---
        auto sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        sliderRep->SetMinimumValue(2.5);
        sliderRep->SetMaximumValue(7.0);
        sliderRep->SetValue(INITIAL_CONTOUR_VALUE);

        sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint1Coordinate()->SetValue(0.02, 0.02);
        sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint2Coordinate()->SetValue(0.1, 0.02);

        m_sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
        m_sliderWidget->SetInteractor(interactor);
        m_sliderWidget->SetRepresentation(sliderRep);
        m_sliderWidget->SetAnimationModeToAnimate();
        m_sliderWidget->EnabledOn();

        // --- Slider callback ---
        struct SliderCallback : public vtkCommand
        {
            static SliderCallback* New() { return new SliderCallback; }

            void Execute(vtkObject* caller, unsigned long, void*) override
            {
                auto sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
                double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
                contourFilter->SetValue(0, value);
                contourFilter->Update();
                sliderWidget->GetInteractor()->GetRenderWindow()->Render();
            }

            vtkContourFilter* contourFilter = nullptr;
        };

        vtkSmartPointer<SliderCallback> sliderCallback = vtkSmartPointer<SliderCallback>::New();
        sliderCallback->contourFilter = m_contourFilter;
        m_sliderWidget->AddObserver(vtkCommand::InteractionEvent, sliderCallback);


    }

private:
    vtkSmartPointer<vtkImageData> m_imageData;

    vtkSmartPointer<vtkContourFilter> m_contourFilter;
    vtkSmartPointer<vtkPolyDataMapper> m_polyDataMapper;
    vtkSmartPointer<vtkActor> m_contourActor;
    vtkSmartPointer<vtkActor> m_outlineActor;
    vtkSmartPointer<vtkSliderWidget> m_sliderWidget;

    static constexpr double X_MIN = -2.0;
    static constexpr double X_MAX = 2.0;
    static constexpr double Y_MIN = -2.0;
    static constexpr double Y_MAX = 2.0;
    static constexpr double Z = 0.0;
    static constexpr double INITIAL_CONTOUR_VALUE = 3.17216;

    void CreateGrid()
    {
        m_imageData = vtkSmartPointer<vtkImageData>::New();
        int m_resolution = 50;
        m_imageData->SetDimensions(m_resolution, m_resolution, 1);

        double spacingX = (X_MAX - X_MIN) / (m_resolution - 1);
        double spacingY = (Y_MAX - Y_MIN) / (m_resolution - 1);
        m_imageData->SetSpacing(spacingX, spacingY, 1.0);
        m_imageData->SetOrigin(X_MIN, Y_MIN, Z);
        m_imageData->AllocateScalars(VTK_DOUBLE, 1);

        // 1) Create an outline of the image data
        auto outline = vtkSmartPointer<vtkOutlineFilter>::New();
        outline->SetInputData(m_imageData);

        // 2) Map it
        auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        outlineMapper->SetInputConnection(outline->GetOutputPort());

        // 3) Create an actor
        m_outlineActor = vtkSmartPointer<vtkActor>::New();
        m_outlineActor->SetMapper(outlineMapper);
        m_outlineActor->GetProperty()->SetColor(0.5, 0.5, 0.5);  // grey
        m_outlineActor->GetProperty()->SetLineWidth(2);
    }

    void SampleField()
    {
        int m_resolution = 50;
        for (int j = 0; j < m_resolution; ++j)
        {
            for (int i = 0; i < m_resolution; ++i)
            {
                double x = X_MIN + i * (X_MAX - X_MIN) / (m_resolution - 1);
                double y = Y_MIN + j * (Y_MAX - Y_MIN) / (m_resolution - 1);
                Vector2d pos(x, y);
                double jacobiValue = CRTBP::JacobiConstant(pos, 0);
                double* pixel = static_cast<double*>(m_imageData->GetScalarPointer(i, j, 0));
                *pixel = jacobiValue;
            }
        }

    }

    void CreateContour()
    {
        m_contourFilter = vtkSmartPointer<vtkContourFilter>::New();
        m_contourFilter->SetInputData(m_imageData);
        m_contourFilter->SetValue(0, INITIAL_CONTOUR_VALUE);
        m_contourFilter->Update();

        m_polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_polyDataMapper->SetInputConnection(m_contourFilter->GetOutputPort());
        m_polyDataMapper->ScalarVisibilityOff();

        m_contourActor = vtkSmartPointer<vtkActor>::New();
        m_contourActor->SetMapper(m_polyDataMapper);
        m_contourActor->GetProperty()->SetColor(1.0, 0.84, 0.0);
        m_contourActor->GetProperty()->SetLineWidth(1.2);
    }


};