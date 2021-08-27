//
// Created by wyz on 2021/8/26.
//

#ifndef VOXER_VOXERRENDERER_HPP
#define VOXER_VOXERRENDERER_HPP
#include <memory>
#include <voxer/Data/Camera.hpp>
#include <voxer/Data/Image.hpp>
#include <voxer/Data/Isosurface.hpp>
#include <voxer/Data/Volume.hpp>
using namespace voxer;
class VoxerRendererImpl;
class VoxerRenderer
{
  public:
    explicit VoxerRenderer(int w,int h);
    ~VoxerRenderer();
    void SetBackGround(float r, float g, float b) noexcept;

    void AddVolume(const std::shared_ptr<Volume> &volume);

    void AddIsoSurface(const std::shared_ptr<Isosurface> &isosurface);
    
    void Render() const;


  private:
    std::unique_ptr<VoxerRendererImpl> impl;
};

#endif // VOXER_VOXERRENDERER_HPP
