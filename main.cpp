#include    <GL/glew.h>
#include    <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include    <iostream>
#include    <vector>
#include    <cmath>
#include    <algorithm>

#include    "shader.h"
#include    "Objects.h"


void    key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if(action == GLFW_PRESS) 
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main (int argc, char *argv[]) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "arcanoid", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);  
    glViewport(0, 0, width, height);


    Shader  shader("./shaders2/default.vs", "./shaders2/default.frag");
    Mesh    *m[] = {new Mesh ("./meshes/cube.mesh"), new Mesh ("./meshes/circle.mesh")};
    Scene   scene (60, 60, shader.Program, m);

    
    float   lastTime = glfwGetTime ();

    
    while (!glfwWindowShouldClose (window)) {

        glfwPollEvents ();
        float   currenTime = glfwGetTime ();
        float   elapsedTime = currenTime - lastTime;
        lastTime = currenTime;
        scene.update(elapsedTime);

        glClear(GL_COLOR_BUFFER_BIT);
        shader.Use();
        scene.Draw();
        glfwSwapBuffers(window);
    }

    glfwTerminate ();


    return 0;
}