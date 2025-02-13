// ======================================================================== //
// Copyright 2018-2019 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "SampleRenderer.h"

// our helper library for window handling
#include "glfWindow/GLFWindow.h"
#include <GL/gl.h>

/*! \namespace osc - Optix Siggraph Course */
namespace osc {

  struct SampleWindow : public GLFCameraWindow
  {
    SampleWindow(const std::string &title,
                 const TriangleMesh &model,
                 const Camera &camera,
                 const float worldScale)
      : GLFCameraWindow(title,camera.from,camera.at,camera.up,worldScale),
        sample(model)
    { 
      std::cout << "gk: calling sample.setCamera in SampleWindow constructor\n";
      sample.setCamera(camera);
      std::cout << "gk: back from sample.setCamera in SampleWindow constructor\n";
    }
    
    virtual void render() override
    {
      if (cameraFrame.modified) {
        std::cout << "gk: calling sample.setCamera in SampleWindow::render\n";
        sample.setCamera(Camera{ cameraFrame.get_from(),
                                 cameraFrame.get_at(),
                                 cameraFrame.get_up() });
        std::cout << "gk: back from sample.setCamera in SampleWindow::render\n";
        cameraFrame.modified = false;
        //gk: moved render here to not be overwhelmed with prinout data and hit position data
        //gk: moving the image in the window will call this again
        sample.render();
        sample.downloadHitCoords(hit_coords.data());
      }
      //gk debug: sample.render();
    }
    
    virtual void draw() override
    {
      sample.downloadPixels(pixels.data());

      if (fbTexture == 0)
        glGenTextures(1, &fbTexture);
      
      glBindTexture(GL_TEXTURE_2D, fbTexture);
      GLenum texFormat = GL_RGBA;
      GLenum texelType = GL_UNSIGNED_BYTE;
      glTexImage2D(GL_TEXTURE_2D, 0, texFormat, fbSize.x, fbSize.y, 0, GL_RGBA,
                   texelType, pixels.data());

      glDisable(GL_LIGHTING);
      glColor3f(1, 1, 1);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, fbTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      
      glDisable(GL_DEPTH_TEST);

      glViewport(0, 0, fbSize.x, fbSize.y);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.f, (float)fbSize.x, 0.f, (float)fbSize.y, -1.f, 1.f);

      glBegin(GL_QUADS);
      {
        glTexCoord2f(0.f, 0.f);
        glVertex3f(0.f, 0.f, 0.f);
      
        glTexCoord2f(0.f, 1.f);
        glVertex3f(0.f, (float)fbSize.y, 0.f);
      
        glTexCoord2f(1.f, 1.f);
        glVertex3f((float)fbSize.x, (float)fbSize.y, 0.f);
      
        glTexCoord2f(1.f, 0.f);
        glVertex3f((float)fbSize.x, 0.f, 0.f);
      }
      glEnd();
    }
    
    virtual void resize(const vec2i &newSize) 
    {
      fbSize = newSize;
      sample.resize(newSize);
      pixels.resize(newSize.x*newSize.y);
      hit_coords.resize(newSize.x*newSize.y);
    }

    vec2i                 fbSize;
    GLuint                fbTexture {0};
    SampleRenderer        sample;
    std::vector<uint32_t> pixels;
    std::vector<float3>   hit_coords;
  };
  
  
  /*! main entry point to this example - initially optix, print hello
    world, then exit */
  extern "C" int main(int ac, char **av)
  {
    try {
      TriangleMesh model;
      // 100x100 thin ground plane
      model.addCube(vec3f(0.f,-1.5f,0.f),vec3f(10.f,.1f,10.f));
      // a unit cube centered on top of that
      model.addCube(vec3f(0.f,0.f,0.f),vec3f(2.f,2.f,2.f));
      model.color = vec3f(.2f,.8f,.2f);
      
      Camera camera = { /*from*/vec3f(-10.f,2.f,-12.f),
                        /* at */vec3f(0.f,0.f,0.f),
                        /* up */vec3f(0.f,1.f,0.f) };
      // something approximating the scale of the world, so the
      // camera knows how much to move for any given user interaction:
      const float worldScale = 10.f;

      SampleWindow *window = new SampleWindow("Optix 7 Course Example",
                                              model,camera,worldScale);
      window->run();
      //std::cout << "gk: calling SampleRenderer->render() in main";
      //window->render();
      //std::cout << "gk: back from SampleRenderer->render()";
      
    } catch (std::runtime_error& e) {
      std::cout << GDT_TERMINAL_RED << "FATAL ERROR: " << e.what()
                << GDT_TERMINAL_DEFAULT << std::endl;
      exit(1);
    }
    return 0;
  }
  
} // ::osc
