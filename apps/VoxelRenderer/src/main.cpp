//
// Created by wyz on 2021/8/26.
//
#include "VoxerRenderer.hpp"
#include <iostream>
int main(){
    try{
        auto dataset = StructuredGrid::Load("foot_256_256_256_uint8.raw");
        auto dataset2= StructuredGrid::Load("aneurism_256_256_256_uint8.raw");
        auto tfcn1 = std::make_shared<TransferFunction>();
        tfcn1->add_point(0.0, 0.0, {0.0, 0.0, 0.0});
        tfcn1->add_point(0.5, 0.0, {0.0, 0.0, 0.0});
        tfcn1->add_point(1.0, 1.0, {0.9, 0.2, 0.5});
        auto tfcn2 = std::make_shared<TransferFunction>();
        tfcn2->add_point(0.0, 0.0, {0.0, 0.0, 0.0});
        tfcn2->add_point(0.5, 0.0, {0.0, 0.0, 0.0});
        tfcn2->add_point(1.0, 1.0, {1.0, 1.0, 1.0});
        VoxerRenderer renderer(1200, 900);
        {
            auto volume = std::make_shared<Volume>();
            volume->tfcn = tfcn1;
            volume->dataset = dataset;
            renderer.AddVolume(volume);

            auto volume2 = std::make_shared<Volume>();
            volume2->tfcn = tfcn2;
            volume2->dataset = dataset2;
            renderer.AddVolume(volume2);
        }
        //not imply correctly for iso_surface and volume mix render
        //just support two volumes or two isosurface
        {
//            auto iso_surface=std::make_shared<Isosurface>();
//            iso_surface->dataset=dataset;
//            iso_surface->value=130.f;
//            iso_surface->color.data={1.f,0.f,0.f};
//            renderer.AddIsoSurface(iso_surface);

//            auto iso_surface2=std::make_shared<Isosurface>();
//            iso_surface2->dataset=dataset2;
//            iso_surface2->value=160.f;
//            iso_surface2->color.data={0.f,1.f,0.f};
//            renderer.AddIsoSurface(iso_surface2);
        }
        renderer.SetBackGround(0.f, 0.f, 0.f);
        renderer.Render();
        return 0;
    }
    catch (const std::exception& err)
    {
        std::cout<<err.what()<<std::endl;
    }
}