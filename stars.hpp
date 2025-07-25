#pragma once

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkSkybox.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include<string>

using namespace std;

class Stars
{
public:
    Stars()
    {

        string imagePath = "./../images/panorama_image.png";

        // Load the image using image reader, I referred VTK documentation for this.
        vtkNew<vtkImageReader2Factory> factory;
        vtkSmartPointer<vtkImageReader2> reader;
        reader.TakeReference(factory->CreateImageReader2(imagePath.c_str()));
        reader->SetFileName(imagePath.c_str());
        reader->Update();



        //Set the image data into the vtkTexture
        texture_->SetInputData(reader->GetOutput());
        texture_->InterpolateOn();
        texture_->MipmapOn();

        //Set the texture object into the vtkSkybox
        skybox_->SetTexture(texture_.GetPointer());
        skybox_->SetProjectionToSphere();
    }

    //Create renderer using reference from earth.hpp
    void InitRenderer(vtkRenderer* renderer)
    {
        renderer->AddActor(skybox_.GetPointer());
        renderer->SetEnvironmentTexture(texture_.GetPointer());
    }

private:
    vtkNew<vtkTexture> texture_;
    vtkNew<vtkSkybox>  skybox_;
};