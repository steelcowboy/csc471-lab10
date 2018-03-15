/**
 * Base code
 * Draws two meshes and one ground plane, one mesh has textures, as well
 * as ground plane.
 * Must be fixed to load in mesh with multiple shapes (dummy.obj)
 */

#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Particle.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

    public:

        WindowManager * windowManager = nullptr;

        // Our shader program
        std::shared_ptr<Program> prog;
        std::shared_ptr<Program> NefProg;

        std::shared_ptr<Shape> shape;
        std::vector<std::shared_ptr<Particle>> particles;

        // CPU array for particles - redundant with particle structure
        // but simple
        int numP = 300;
        GLfloat points[900];
        GLfloat pointColors[1200];

        GLuint pointsbuffer;
        GLuint colorbuffer;

        // Contains vertex information for OpenGL
        GLuint VertexArrayID;

        // OpenGL handle to texture data
        shared_ptr<Texture> texture;

        int gMat = 0;

        // Display time to control fps
        float t0_disp = 0.0f;
        float t_disp = 0.0f;

        bool keyToggles[256] = { false };
        float t = 0.0f; //reset in init
        float h = 0.01f;
        glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);

        float camRot = 0;

        const vec3 light = vec3(-5, 5, 2);


        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            keyToggles[key] = ! keyToggles[key];

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }

            if (key == GLFW_KEY_A && action == GLFW_PRESS)
            {
                camRot -= 0.314f;
            }
            if (key == GLFW_KEY_D && action == GLFW_PRESS)
            {
                camRot += 0.314f;
            }
        }

        void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
        {
        }

        void mouseCallback(GLFWwindow *window, int button, int action, int mods)
        {
            double posX, posY;

            if (action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &posX, &posY);
                cout << "Pos X " << posX << " Pos Y " << posY << endl;
            }
        }

        void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
        }

        void resizeCallback(GLFWwindow *window, int width, int height)
        {
            CHECKED_GL_CALL(glViewport(0, 0, width, height));
        }

        //code to set up the two shaders - a diffuse shader and texture mapping
        void init(const std::string& resourceDirectory)
        {
            int width, height;
            glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
            GLSL::checkVersion();

            // Set background color.
            CHECKED_GL_CALL(glClearColor(.12f, .34f, .56f, 1.0f));

            // Enable z-buffer test.
            CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));

            // Enable alpha blending
            CHECKED_GL_CALL(glEnable(GL_BLEND));
            CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            CHECKED_GL_CALL(glPointSize(14.0f));
            
            // Initialize the GLSL program.
            prog = make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(
                    resourceDirectory + "/lab10_vert.glsl",
                    resourceDirectory + "/lab10_frag.glsl");
            if (! prog->init())
            {
                std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
                exit(1);
            }
            prog->addUniform("P");
            prog->addUniform("V");
            prog->addUniform("M");
            prog->addUniform("alphaTexture");
            prog->addAttribute("vertPos");

            NefProg = make_shared<Program>();
            NefProg->setVerbose(true);
            NefProg->setShaderNames(resourceDirectory + "/phong_vert.glsl", resourceDirectory + "/phong_frag.glsl");
            if (NefProg->init()) {
                //exit(1);
            }
            NefProg->addUniform("P");
            NefProg->addUniform("V");
            NefProg->addUniform("M");
            NefProg->addUniform("uMesh");
            NefProg->addUniform("uLight");
            NefProg->addUniform("MatAmb");
            NefProg->addUniform("MatDif");
            NefProg->addUniform("MatSpec");
            NefProg->addUniform("shine");
            NefProg->addAttribute("vertPos");
            NefProg->addAttribute("vertNor");
        }

        // Code to load in the three textures
        void initTex(const std::string& resourceDirectory)
        {
            texture = make_shared<Texture>();
            texture->setFilename(resourceDirectory + "/alpha.bmp");
            texture->init();
            texture->setUnit(0);
            texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        }

        void initParticles()
        {
            int n = numP;

            for (int i = 0; i < n; ++ i)
            {
                auto particle = make_shared<Particle>();
                particles.push_back(particle);
                particle->load();
            }
        }

        void initGeom(const std::string& resourceDirectory)
        {
            // generate the VAO
            CHECKED_GL_CALL(glGenVertexArrays(1, &VertexArrayID));
            CHECKED_GL_CALL(glBindVertexArray(VertexArrayID));

            // generate vertex buffer to hand off to OGL - using instancing
            CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
            // set the current state to focus on our vertex buffer
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
            // actually memcopy the data - only do this once
            CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));

            CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
            // set the current state to focus on our vertex buffer
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
            // actually memcopy the data - only do this once
            CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));

            shape = make_shared<Shape>();
            shape->loadMesh(resourceDirectory + "/Nefertiti-10K.obj");
            shape->resize();
            shape->init();
        }

        // Note you could add scale later for each particle - not implemented
        void updateGeom()
        {
            glm::vec3 pos;
            glm::vec4 col;

            // go through all the particles and update the CPU buffer
            for (int i = 0; i < numP; i++)
            {
                pos = particles[i]->getPosition();
                col = particles[i]->getColor();
                points[i * 3 + 0] = pos.x;
                points[i * 3 + 1] = pos.y;
                points[i * 3 + 2] = pos.z;
                pointColors[i * 4 + 0] = col.r + col.a / 10.f;
                pointColors[i * 4 + 1] = col.g + col.g / 10.f;
                pointColors[i * 4 + 2] = col.b + col.b / 10.f;
                pointColors[i * 4 + 3] = col.a;
            }

            // update the GPU data
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
            CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
            CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
            CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
            CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        /* note for first update all particles should be "reborn"
         * which will initialize their positions */
        void updateParticles()
        {
            // update the particles
            for (auto particle : particles)
            {
                particle->update(t, h, g, keyToggles);
            }
            t += h;

            // Sort the particles by Z
            auto temp = make_shared<MatrixStack>();
            temp->rotate(camRot, vec3(0, 1, 0));

            ParticleSorter sorter;
            sorter.C = temp->topMatrix();
            std::sort(particles.begin(), particles.end(), sorter);
        }

        void render()
        {
            // Get current frame buffer size.
            int width, height;
            glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
            glViewport(0, 0, width, height);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Clear framebuffer.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float aspect = width / (float) height;

            // Create the matrix stacks
            auto P = make_shared<MatrixStack>();
            auto V = make_shared<MatrixStack>();
            auto M = make_shared<MatrixStack>();
            // Apply perspective projection.
            P->pushMatrix();
            P->perspective(45.0f, aspect, 0.01f, 100.0f);

            // camera rotate
            V->pushMatrix();
            V->rotate(camRot, vec3(0, 1, 0));

            M->pushMatrix();
            M->loadIdentity();


            // Draw
            NefProg->bind();
            M->pushMatrix();
            {
                M->translate(vec3(0, 0, -2.2));
                M->rotate(radians(-90.f), vec3(1, 0, 0));
                glUniformMatrix4fv(NefProg->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()) );
                glUniformMatrix4fv(NefProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
                glUniform3f(NefProg->getUniform("uLight"), (float)light.x, (float)light.y, (float)light.z);

                glUniformMatrix4fv(NefProg->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));

                //glUniform3f(NefProg->getUniform("MatAmb"), 0.3294f, 0.2235f, 0.02745f);
                //glUniform3f(NefProg->getUniform("MatDif"), 0.75164f, 0.60648f, 0.22648f);
                //glUniform3f(NefProg->getUniform("MatSpec"), 0.628281f, 0.555802f, 0.366065f);
                //glUniform1f(NefProg->getUniform("shine"), 0.4);
                glUniform3f(NefProg->getUniform("MatAmb"), 0.3294f, 0.2235f, 0.02745f);
                glUniform3f(NefProg->getUniform("MatDif"), 0.7804f, 0.5686f, 0.11373f);
                glUniform3f(NefProg->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
                glUniform1f(NefProg->getUniform("shine"), 27.9);
                shape->draw(NefProg);
            }
            M->popMatrix();

            NefProg->unbind();

            prog->bind();

            updateParticles();
            updateGeom();

            texture->bind(prog->getUniform("alphaTexture"));
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix())));
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));

            CHECKED_GL_CALL(glEnableVertexAttribArray(0));
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
            CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0));

            CHECKED_GL_CALL(glEnableVertexAttribArray(1));
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
            CHECKED_GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0));

            CHECKED_GL_CALL(glVertexAttribDivisor(0, 1));
            CHECKED_GL_CALL(glVertexAttribDivisor(1, 1));
            // Draw the points !
            CHECKED_GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, numP));

            CHECKED_GL_CALL(glVertexAttribDivisor(0, 0));
            CHECKED_GL_CALL(glVertexAttribDivisor(1, 0));
            CHECKED_GL_CALL(glDisableVertexAttribArray(0));
            CHECKED_GL_CALL(glDisableVertexAttribArray(1));
            prog->unbind();

            // Pop matrix stacks.
            M->popMatrix();
            V->popMatrix();
            P->popMatrix();
        }

};

int main(int argc, char **argv)
{
    // Where the resources are loaded from
    std::string resourceDir = "../resources";

    if (argc >= 2)
    {
        resourceDir = argv[1];
    }

    Application *application = new Application();

    // Your main will always include a similar set up to establish your window
    // and GL context, etc.

    WindowManager *windowManager = new WindowManager();
    windowManager->init(512, 512);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;

    // This is the code that will likely change program to program as you
    // may need to initialize or set up different data and state

    application->init(resourceDir);
    application->initTex(resourceDir);
    application->initParticles();
    application->initGeom(resourceDir);

    // Loop until the user closes the window.
    while (! glfwWindowShouldClose(windowManager->getHandle()))
    {
        // Render scene.
        application->render();

        // Swap front and back buffers.
        glfwSwapBuffers(windowManager->getHandle());
        // Poll for and process events.
        glfwPollEvents();
    }

    // Quit program.
    windowManager->shutdown();
    return 0;
}
