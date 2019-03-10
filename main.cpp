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



struct  Mesh {

    GLfloat     *vertices{nullptr};
    GLuint      *indices{nullptr};
    GLuint      vSize{0};
    GLuint      iSize{0};
  
    

    void Draw () {
        glDrawElements (GL_TRIANGLES, iSize/sizeof(GLuint), GL_UNSIGNED_INT, (GLvoid*)0);
    }
};

class Block {
    public:
        Block (glm::vec2 p, glm::vec2 s) : pos(p, 0.0f), scl(s, 1.0f) {} 
        ~Block () {} 
        
        bool        alive{true};
        glm::vec3   pos;
        glm::vec3   scl;
    
};

enum    DIRECTION {LEFT, RIGHT};

class Player : public Block{
    public:
        Player (glm::vec2 p, glm::vec2 s) : Block(p, s) {}
        ~Player () {}

        float   velocity{1.5f};
        
};

class Ball {
    public:
        Ball (glm::vec2 p, float r) : pos(p, 0.0f), radius(r) {}
    
        glm::vec3   pos;
        glm::vec3   velocity {0.0f, 0.0f, 0.0f};
        bool        runned{false};
        float       vel_mag{1.5f};
        float       vel_angle{M_PI/4};
        float       radius;
};

bool    keys[1024];
void    key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
}

int main (int argc, char *argv[]) {


    glfwInit ();
    glfwWindowHint (GLFW_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_VERSION_MINOR, 3);

    GLFWwindow  *window = glfwCreateWindow (800, 600 ,"demo", nullptr, nullptr);
    glfwMakeContextCurrent (window);

    glfwSetKeyCallback (window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit ();

    int width, height;
    glfwGetFramebufferSize (window, &width, &height);
    glViewport (0, 0, width, height);


    Shader  shader("./shaders/default.vs", "./shaders/default.frag");

    GLfloat     cubeVertices[] = {
        0.5f,  0.5f, 0.0f,  // Верхний правый угол 
        0.5f, -0.5f, 0.0f,  // Нижний правый угол
        -0.5f, -0.5f, 0.0f,  // Нижний левый угол
        -0.5f,  0.5f, 0.0f,
         
    };

    GLuint      cubeIndices[] = {
        0, 1, 3,   // Первый треугольник
        1, 2, 3 
    };

    const int   n = 21;
    GLfloat     circleVertices [3*n] = {0.0f, 0.0f, 0.0f};
    for (int i = 1; i < n; i++) {
        circleVertices[3*i]   = 0.5f*cos(2*M_PI/(n-1)*i);
        circleVertices[3*i+1] = 0.5f*sin(2*M_PI/(n-1)*i);
        circleVertices[3*i+2] = 0.0f;
    }
    GLuint      circleIndices[3*(n-1)];
    for (int i = 0; i < n-1; i++) {
        circleIndices[3*i]   = 0;
        circleIndices[3*i+1] = i+1;
        circleIndices[3*i+2] = (i+2)%n + (i+2)/n;
    }

    Mesh        cubeMesh = {cubeVertices, cubeIndices, sizeof(cubeVertices), sizeof(cubeIndices)};
    glm::vec2   cubeScale (0.3f, 0.2f);
    std::vector<Block>  blockList;
    

    GLuint      BlocksRow = 7;
    GLuint      BlocksCol = 15;

    float   GridScaleX = ((BlocksCol-1)*1.1 + 1)*cubeScale.x;
    float   GridScaleY = ((BlocksRow-1)*1.1 + 1)*cubeScale.y;

    float   SceneX = 4*cubeScale.x + GridScaleX;
    float   SceneY = 10*cubeScale.y + GridScaleY;
    

    for (int i = 0; i < BlocksRow; i++) {
        float y = cubeScale.y*1.1f*i + 6.5f*cubeScale.y;
        for (int j = 0; j < BlocksCol; j++) {
            float x = cubeScale.x*1.1f*j + 2.5f*cubeScale.x;
            blockList.push_back (*(new Block(glm::vec2(x, y), cubeScale)));
        }
    }

    float       radius = 0.05f;
    Player      player (glm::vec2(SceneX/2, 0.04f), glm::vec2(0.5f, 0.04f));
    Ball        ball (glm::vec2(SceneX/2, player.scl.y + radius), radius);
    
    // blockList.push_back (*(new Block(glm::vec2(0,0), cubeScale)));
    // blockList.push_back (*(new Block(glm::vec2(cubeScale.x,cubeScale.y), cubeScale)));

    GLuint  blocksVBO, blocksEBO, blocksVAO;
    glGenVertexArrays (1, &blocksVAO);
    glGenBuffers (1, &blocksVBO);
    glGenBuffers (1, &blocksEBO);

    glBindVertexArray (blocksVAO);
        glBindBuffer (GL_ARRAY_BUFFER, blocksVBO);
        glBufferData (GL_ARRAY_BUFFER, cubeMesh.vSize, cubeMesh.vertices, GL_STATIC_DRAW); 

        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, blocksEBO);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, cubeMesh.iSize, cubeMesh.indices, GL_STATIC_DRAW);

        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray (0);

    glBindVertexArray (0);


    Mesh    circleMesh = {circleVertices, circleIndices, sizeof(circleVertices), sizeof(circleIndices)};
    GLuint  circleVBO, circleEBO, circleVAO;
    glGenVertexArrays (1, &circleVAO);
    glGenBuffers (1, &circleVBO);
    glGenBuffers (1, &circleEBO);

    glBindVertexArray (circleVAO);
        glBindBuffer (GL_ARRAY_BUFFER, circleVBO);
        glBufferData (GL_ARRAY_BUFFER, circleMesh.vSize, circleMesh.vertices, GL_STATIC_DRAW); 

        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, circleEBO);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, circleMesh.iSize, circleMesh.indices, GL_STATIC_DRAW);

        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray (0);

    glBindVertexArray (0);

    float   lastTime = glfwGetTime();

    while (!glfwWindowShouldClose (window)) {
        glfwPollEvents ();

        float   currentTime = glfwGetTime ();
        float   elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        float dx_player = player.velocity*elapsedTime;
        if (keys[GLFW_KEY_LEFT]) {
            float newPos = player.pos.x - dx_player;
            player.pos.x = ((newPos - player.scl.x/2 > 0) ? newPos : player.pos.x);
        }
        if (keys[GLFW_KEY_RIGHT]) {
            float newPos = player.pos.x + dx_player;
            player.pos.x = ((newPos + player.scl.x/2 < SceneX) ? newPos : player.pos.x);
        }

        if (keys[GLFW_KEY_SPACE]) {
            if (!ball.runned) {
                ball.runned = true;
                ball.velocity = glm::vec3(ball.vel_mag*cos(ball.vel_angle), ball.vel_mag*sin(ball.vel_angle), 0.0f);
            }
        }
        
        if (ball.runned) {
            auto    dx_ball = ball.velocity*elapsedTime;
            auto    ballPos = ball.pos + dx_ball;


            if (ballPos.x - ball.radius > 0 && ballPos.x + ball.radius < SceneX) {
                ball.pos = ballPos;
            } else if (ballPos.x - ball.radius < 0) {
                if (ball.velocity.x < 0)
                    ball.velocity.x *= -1;
            } else  if (ball.velocity.x > 0)
                        ball.velocity.x *= -1;


            if (ballPos.y - ball.radius > 0 && ballPos.y + ball.radius < SceneY) {
                ball.pos = ballPos;
            } else if (ballPos.y - ball.radius < 0) {
                if (ball.velocity.y < 0)
                    ball.pos.x = player.pos.x;
                    ball.pos.y = player.scl.y + radius;
                    ball.runned = false;
            } else  if (ball.velocity.y > 0)
                        ball.velocity.y *= -1;


            if (ball.pos.y - ball.radius < player.pos.y+player.scl.y/2) {
                if (ball.pos.x < player.pos.x+player.scl.x/2 && ball.pos.x > player.pos.x-player.scl.x/2) {
                    if (ball.velocity.y < 0)
                        ball.velocity.y *= -1;
                }
            }


            if (blockList.size() > 0) {
                for (auto &block : blockList) {
                    glm::vec3 pv[] = {
                        glm::vec3 (block.pos.x-block.scl.x/2, block.pos.y-block.scl.y/2, 0.f),
                        glm::vec3 (block.pos.x+block.scl.x/2, block.pos.y-block.scl.y/2, 0.f),
                        glm::vec3 (block.pos.x-block.scl.x/2, block.pos.y+block.scl.y/2, 0.f),
                        glm::vec3 (block.pos.x+block.scl.x/2, block.pos.y+block.scl.y/2, 0.f)
                    };

                    for (int i = 0; i < 4; i+=2) {
                        float a = glm::distance (pv[(i+1)%4], pv[i]);
                        float b = glm::distance (pv[(i+1)%4], ball.pos);
                        float c = glm::distance (pv[i], ball.pos);
                        float p = (a+b+c)/2;

                        if (b*b < ball.radius*ball.radius || c*c < ball.radius*ball.radius) {
                            block.alive = false;
                            ball.velocity.y*=-1;
                            break;
                        }

                        float h2 = 4*p*(p-a)*(p-b)*(p-c) /(a*a);
                        if (h2 <= ball.radius*ball.radius && ball.pos.x >= pv[i].x && ball.pos.x <= pv[(i+1)%4].x) {
                            block.alive = false;
                            ball.velocity.y*=-1;
                            break;
                        }
                    }

                    for (int i = 1; i <= 4; i+=3) {
                        float a = glm::distance (pv[(i+2)%4], pv[i%4]);
                        float b = glm::distance (pv[(i+2)%4], ball.pos);
                        float c = glm::distance (pv[i%4], ball.pos);
                        float p = (a+b+c)/2;

                        if (b*b < ball.radius*ball.radius || c*c < ball.radius*ball.radius) {
                            block.alive = false;
                            ball.velocity.x*=-1;
                            break;
                        }

                        float h2 = 4*p*(p-a)*(p-b)*(p-c) /(a*a);
                        if (h2 <= ball.radius*ball.radius && ball.pos.y >= pv[i%4].y && ball.pos.y <= pv[(i+2)%4].y) {
                            block.alive = false;
                            ball.velocity.x*=-1;
                            break;
                        }
                    }
                }

                auto iter = std::remove_if (blockList.begin(), blockList.end(), [](Block o){return !o.alive;});
                if (iter != blockList.end()) {
                    blockList.erase(iter);
                }
            }
            
        }
        else {
            ball.pos.x = player.pos.x;
            ball.pos.y = player.scl.y + radius;
        }
        glClear (GL_COLOR_BUFFER_BIT);

        shader.Use();

        glBindVertexArray (blocksVAO);       

        for (auto &block : blockList) {
            glm::mat4   model = glm::translate (glm::mat4(1.f), block.pos);//= glm::scale (glm::mat4(1.0f), block.scl);
            model = glm::scale (model, block.scl);//= glm::translate (model, block.pos);
            glm::mat4   view = glm::translate (glm::mat4(1.f), glm::vec3(-1.f , -1.f, 0.0f)); 
            view  = glm::scale (view, glm::vec3(  2/SceneX,
                                                  2/SceneY,
                                                  1.0f)
                                                  );
            
            GLuint  modelLocation = glGetUniformLocation (shader.Program, "model");
            glUniformMatrix4fv (modelLocation, 1, GL_FALSE, glm::value_ptr(model));
            
            GLuint  viewLocation = glGetUniformLocation (shader.Program, "view");
            glUniformMatrix4fv (viewLocation, 1, GL_FALSE, glm::value_ptr(view));

            glDrawElements (GL_TRIANGLES, cubeMesh.iSize/sizeof(GLuint), GL_UNSIGNED_INT, 0);
        }

        glm::mat4   model = glm::translate (glm::mat4(1.f), player.pos);//= glm::scale (glm::mat4(1.0f), block.scl);
        model = glm::scale (model, player.scl);//= glm::translate (model, block.pos);
        glm::mat4   view = glm::translate (glm::mat4(1.f), glm::vec3(-1.f , -1.f, 0.0f)); 
        view  = glm::scale (view, glm::vec3(  2/SceneX,
                                                2/SceneY,
                                                1.0f)
                                                );
        
        GLuint  modelLocation = glGetUniformLocation (shader.Program, "model");
        glUniformMatrix4fv (modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        
        GLuint  viewLocation = glGetUniformLocation (shader.Program, "view");
        glUniformMatrix4fv (viewLocation, 1, GL_FALSE, glm::value_ptr(view));

        glDrawElements (GL_TRIANGLES, cubeMesh.iSize/sizeof(GLuint), GL_UNSIGNED_INT, 0);

        glBindVertexArray (0);


        glBindVertexArray (circleVAO);

        model = glm::translate (glm::mat4(1.f), ball.pos);//= glm::scale (glm::mat4(1.0f), block.scl);
        model = glm::scale (model, glm::vec3(2*ball.radius, 2*ball.radius, 1.0f));//= glm::translate (model, block.pos);
        
        modelLocation = glGetUniformLocation (shader.Program, "model");
        glUniformMatrix4fv (modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        
        viewLocation = glGetUniformLocation (shader.Program, "view");
        glUniformMatrix4fv (viewLocation, 1, GL_FALSE, glm::value_ptr(view));

        glDrawElements (GL_TRIANGLES, circleMesh.iSize/sizeof(GLuint), GL_UNSIGNED_INT, 0);

        glBindVertexArray (0);

        glfwSwapBuffers(window);
    }    

    glfwTerminate();
    return 0;
}