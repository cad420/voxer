//
// Created by wyz on 2021/8/26.
//

#include "VoxerRenderer.hpp"
#include <voxer/Renderers/VolumeRenderer.hpp>
#include <SDL.h>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include "Camera.hpp"
#define DEBUG
#ifdef DEBUG
#define SDL_EXPR(exec)                                                                                                 \
    exec;                                                                                                              \
    {                                                                                                                  \
        std::string __err_str = SDL_GetError();                                                                              \
        if (__err_str.length() > 0)                                                                                          \
        {                                                                                                              \
            spdlog::error("SDL call error: {0} in {1} function: {2} line: {3}",__err_str,__FILE__,__FUNCTION__,__LINE__);\
        }                                                                                                              \
    }

#define SDL_CHECK                                                                                                      \
    {                                                                                                                  \
        std::string __err_str = SDL_GetError();                                                                              \
        if (__err_str.length() > 0)                                                                                          \
        {                                                                                                              \
            spdlog::error("SDL call error: {0} in {1} before line: {2}",__err_str,__FILE__,__LINE__);\
        }                                                                                                               \
    }

#else
#define SDL_EXPR(exec) exec
#define SCL_CHECK
#endif

class VoxerRendererImpl{
  public:

    VoxerRendererImpl(int w,int h);
    ~VoxerRendererImpl();
    void SetBackGround(float r, float g, float b) noexcept;

    void AddVolume(const std::shared_ptr<Volume> &volume);

    void AddIsoSurface(const std::shared_ptr<Isosurface> &isosurface);

    void Render() const;
  private:
    void initSDL();
  private:
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    int window_w,window_h;
    std::unique_ptr<VolumeRenderer> renderer;


};
VoxerRendererImpl::VoxerRendererImpl(int w, int h)
:window_w(w),window_h(h)
{
    renderer=std::make_unique<VolumeRenderer>("opengl");
    initSDL();
}
void VoxerRendererImpl::SetBackGround(float r, float g, float b) noexcept
{
    renderer->set_background(r,g,b);
}
void VoxerRendererImpl::AddVolume(const std::shared_ptr<Volume> &volume)
{
    renderer->add_volume(volume);
}
void VoxerRendererImpl::AddIsoSurface(const std::shared_ptr<Isosurface> &isosurface)
{
    renderer->add_isosurface(isosurface);
}
void VoxerRendererImpl::Render() const
{
    SDL_Rect rect{0,0,(int)window_w,(int)window_h};
    SDL_Texture* texture=SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,window_w,window_h);

    SDL_CHECK
    bool exit=false;
    auto process_event=[&exit,this](){
      static SDL_Event event;
      static control::TrackBallCamera trackball_camera(128.f,window_w,window_h,{128.f,128.f,128.f});
      static bool left_mouse_button_press;
      while(SDL_PollEvent(&event)){
          switch(event.type){
              case SDL_QUIT:{
                  exit=true;
                  break;
              }
              case SDL_MOUSEBUTTONDOWN:{
                  if(event.button.button==1){
                      left_mouse_button_press=true;
                      trackball_camera.processMouseButton(control::CameraDefinedMouseButton::Left,true,event.button.x,window_h-event.button.y);
                  }
                  break;
              }
              case SDL_MOUSEBUTTONUP:{
                  if(event.button.button==1){
                      left_mouse_button_press=false;
                      trackball_camera.processMouseButton(control::CameraDefinedMouseButton::Left,false,event.button.x,window_h-event.button.y);
                  }
                  break;
              }
              case SDL_MOUSEMOTION:{
                  if(left_mouse_button_press){

                      trackball_camera.processMouseMove(event.button.x,window_h-event.button.y);
                  }
                  break;
              }
              case SDL_MOUSEWHEEL:{
                  trackball_camera.processMouseScroll(event.wheel.y);
                  break;
              }
          }
      }//end of SDL_PollEvent
      Camera camera;
      camera.width=window_w;
      camera.height=window_h;
      auto pos=trackball_camera.getCameraPos();
//      spdlog::info("camera pos: {0} {1} {2} {3}",pos.x,pos.y,pos.z,(pos.x-128.f)*(pos.x-128.f)+(pos.y-128.f)*(pos.y-128.f)+(pos.z-128.f)*(pos.z-128.f));
      auto lookat=trackball_camera.getCameraLookAt();
      auto up=trackball_camera.getCameraUp();
      camera.pos={pos.x,pos.y,pos.z};
      camera.up={up.x,up.y,up.z};
      camera.target={lookat.x,lookat.y,lookat.z};
      renderer->set_camera(camera);
    };

    while(!exit){
        process_event();
        renderer->render();
        auto frame=renderer->get_colors();
        SDL_UpdateTexture(texture,NULL,frame.data.data(),window_w*4);
        SDL_RenderClear(sdl_renderer);
        SDL_RenderCopy(sdl_renderer,texture, nullptr,&rect);
        SDL_RenderPresent(sdl_renderer);

        SDL_CHECK
    }
}
void VoxerRendererImpl::initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        throw std::runtime_error("SDL could not initialize");
    }
    SDL_EXPR(sdl_window = SDL_CreateWindow("VoxerRenderer", 100, 100, window_w, window_h, SDL_WINDOW_SHOWN));
    SDL_EXPR(sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED));

}
VoxerRendererImpl::~VoxerRendererImpl()
{
    SDL_EXPR(SDL_DestroyRenderer(sdl_renderer));
    SDL_EXPR(SDL_DestroyWindow(sdl_window));
    SDL_EXPR(SDL_Quit());
}

//-------------------------------------------------------------
VoxerRenderer::VoxerRenderer(int w, int h)
{
    impl=std::make_unique<VoxerRendererImpl>(w,h);
}
void VoxerRenderer::SetBackGround(float r, float g, float b) noexcept
{
    impl->SetBackGround(r,g,b);
}
void VoxerRenderer::AddVolume(const std::shared_ptr<Volume> &volume)
{
    impl->AddVolume(volume);
}
void VoxerRenderer::AddIsoSurface(const std::shared_ptr<Isosurface> &isosurface)
{
    impl->AddIsoSurface(isosurface);
}
void VoxerRenderer::Render() const
{
    impl->Render();
}
VoxerRenderer::~VoxerRenderer()
{
    impl.reset();
}
