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

    Scene   scene (60, 60, shader.Program);
    Player  player (&scene);
    Ball    ball (&scene);
    Block   block(glm::vec2(scene.SizeX/2, scene.SizeY/2), &scene);

    Mesh    cubeMesh ("./meshes/cube.mesh");
    Mesh    circleMesh ("./meshes/circle.mesh");


    Perfab  pPlayer (&player, &cubeMesh);
    Perfab  pBall (&ball, &circleMesh);
    Perfab  pBloc (&block, &cubeMesh);
    

    float   lastTime = glfwGetTime ();

    
    while (!glfwWindowShouldClose (window)) {

        glfwPollEvents ();

        float   currenTime = glfwGetTime ();
        float   elapsedTime = currenTime - lastTime;
        lastTime = currenTime;

        player.update(elapsedTime);
        ball.update (elapsedTime);
        block.update(elapsedTime);
        

        glClear(GL_COLOR_BUFFER_BIT);

        shader.Use();


        // glBindVertexArray (circleMesh.VAO);
        
        // glm::mat4   trans = glm::scale (glm::mat4(1.0f), ball.scl);

        // glm::mat4   view = glm::scale (trans, glm::vec3(1.0/scene.SizeX, 1.0/scene.SizeY, 1.0));
        // glm::mat4   rescl = glm::scale (glm::mat4(1.f), glm::vec3 (1.0f, (float)8/6, 1.0));

        // GLuint      resclLoc = glGetUniformLocation (shader.Program, "rescl");
        // glUniformMatrix4fv (resclLoc, 1, GL_FALSE, glm::value_ptr(rescl));

        // GLuint     transLoc = glGetUniformLocation (shader.Program, "trans");
        // glUniformMatrix4fv (transLoc, 1, GL_FALSE, glm::value_ptr(view));

        // circleMesh.Draw();
        // glBindVertexArray (0);

        pPlayer.Draw();
        pBall.Draw ();
        pBloc.Draw();

        pBall.Draw();
        

        glfwSwapBuffers(window);
    }

    glfwTerminate ();


    return 0;
}