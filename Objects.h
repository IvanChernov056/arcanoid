#ifndef     OBJECTS_H
#define     OBJECTS_H


#include    <GL/glew.h>
#include    <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include    <iostream>
#include    <fstream>
#include    <sstream>
#include    <vector>
#include    <cmath>
#include    <algorithm>

class   Object;
struct  Mesh;
class   Block;
class   Player;
class   Ball;

const float BlockX = 3, BlockY = 1,
            PlayerX = 10, PlayerY = 1,
            PlayerVel = 30.f,
            BallR = 1.0f,
            VelMag = 40.0f,
            VelX = VelMag*cos(M_PI_4), VelY = VelMag*sin(M_PI_4);
bool    keys[1024];

struct  Scene {

    GLuint  SizeX, SizeY;
    GLuint  BlocksRow{20}, BlocksCol{15};

    Player  *player{nullptr};
    Ball    *ball{nullptr};

    Mesh    *blockMesh{nullptr};

    std::vector<Block> blockList;
    GLuint      shaderProgram;
    glm::mat4   view;

    Scene (GLuint sx, GLuint sy, GLuint sh_prog, Mesh **m);

    void Draw();
    void update(float elapsedTime);
    void resetRaund ();
    void resetGame ();
};

struct  Mesh {

    GLfloat     *vertices{nullptr};
    GLuint      *indices{nullptr};
    GLuint      vSize{0};
    GLuint      iSize{0};
  
    GLuint VBO, VAO, EBO;
    Mesh(){}
    Mesh (GLfloat *a, GLuint *b, GLuint c, GLuint d) : vSize(c), iSize(d) {
        auto    vLen = vSize/sizeof(GLfloat);
        auto    iLen = iSize/sizeof(GLuint);
        vertices = new GLfloat[vLen];
        indices  = new GLuint[iLen];
        for (int i = 0; i < vLen; i++) {
            vertices[i] = a[i]; 
        }
        for (int i = 0; i < iLen; i++) {
            indices[i] = b[i];
        }

        Init();
    }
    Mesh (const char *file_name) {
        load (file_name);
        Init();
    }
    ~Mesh () {
        if (vertices != nullptr)
            delete []vertices;
        if (indices != nullptr)
            delete []indices;
        Free();
    }

    
    void Init () {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers (1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vSize, vertices, GL_STATIC_DRAW);

        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, iSize, indices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
    void Free() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void Draw () {
        glDrawElements (GL_TRIANGLES, iSize/sizeof(GLuint), GL_UNSIGNED_INT, (GLvoid*)0);
    }
    void load (const char *file_name) {
            std::ifstream       fStream (file_name);

            std::vector<glm::vec3>   vVector;
            std::vector<glm::uvec3>  iVector;

            while (!fStream.eof()) {
                char    line[128];
                fStream.getline (line, 128);

                std::stringstream   s;
                s << line;

                char        ctmp;
                glm::vec3   v;
                glm::uvec3  i;

                switch (line[0]) {
                    case 'v' :
                        s >> ctmp >> v.x >> v.y >> v.z;
                        vSize ++;
                        vVector.push_back(v);
                        break;
                    case 'f' :
                        s >> ctmp >> i.x >> i.y >> i.z;
                        iSize ++;
                        iVector.push_back(i);
                        break; 
                    
                    default : break;
                }
            }

            vertices = new GLfloat[3*vSize];
            indices  = new GLuint[3*iSize];

            for (int j = 0; j < vSize; j++) {
                vertices[3*j]   = vVector[j].x;
                vertices[3*j+1] = vVector[j].y;
                vertices[3*j+2] = vVector[j].z;
            }

            for (int j = 0; j < iSize; j++) {
                indices[3*j]   = iVector[j].x;
                indices[3*j+1] = iVector[j].y;
                indices[3*j+2] = iVector[j].z;
            }
    
            vSize *= 3*sizeof(GLfloat);
            iSize *= 3*sizeof(GLuint);
        }
};

class Object {

    public:
        glm::vec3   pos;
        glm::vec3   scl;
        Scene       *scene{nullptr};
        Mesh        *mesh{nullptr};

        Object (Scene *sc, Mesh *m) : scene(sc), mesh(m) {}

        virtual void update (float elapsedTime) {}

        void    Draw () {
            glBindVertexArray(mesh->VAO);

            glm::mat4   model = glm::translate (glm::mat4(1.0), pos);
            model = glm::scale (model, scl);

            GLuint  modUnifLoc = glGetUniformLocation (scene->shaderProgram, "model");
            glUniformMatrix4fv (modUnifLoc, 1, GL_FALSE, glm::value_ptr(model));

            GLuint  viewUnifLoc = glGetUniformLocation (scene->shaderProgram, "view");
            glUniformMatrix4fv (viewUnifLoc, 1, GL_FALSE, glm::value_ptr(scene->view));

            mesh->Draw();

            glBindVertexArray(0);
        }
};

enum DIRECTION {LEFT=-1, RIGHT=1};
class   Player : public Object {

    public:

        Player (Scene *sc, Mesh *m) : Object (sc, m) {
            pos = glm::vec3 (scene->SizeX/2, PlayerY/2, 0.0f);
            scl = glm::vec3 (PlayerX, PlayerY, 0.0f);
        }

        void    update (float elapsedTime) {
            if (keys[GLFW_KEY_LEFT]) {
                move(LEFT, elapsedTime);
            } else if (keys[GLFW_KEY_RIGHT]) {
                move(RIGHT, elapsedTime);
            }
        }

        void move (DIRECTION direct, float elapsedTime) {
            float   dx = PlayerVel*elapsedTime*direct;
            float   newPos = pos.x + dx;
            
            if (newPos - scl.x/2 >= 0 && newPos + scl.x/2 <= scene->SizeX)
                pos.x = newPos;
        }

        void reset () {
            pos = glm::vec3 (scene->SizeX/2, PlayerY/2, 0.0f);
        }

};

class   Ball : public Object {

    public:

        glm::vec3   vel{glm::vec3(VelX,VelY,0.0f)};
        float       R{BallR};
        float       v_mag {VelMag};
        bool        runned{false};

        Ball (Scene *sc, Mesh *m) : Object(sc, m) {
            pos = scene->player->pos + glm::vec3 (0, R + scene->player->scl.y/2, 0.0);
            scl = 2.0f*glm::vec3 (R, R, 0.0);
        }

        void update (float elapsedTime)override {
            if (!runned) {
                if (keys[GLFW_KEY_SPACE]) {
                    runned = true;
                }
                pos.x = scene->player->pos.x;
            } else {
                move(elapsedTime);
            }
        }

        void move(float elapsedTime) {
            glm::vec3   dx = elapsedTime*vel;
            glm::vec3   newPos = pos + dx;

            float dr;
            if ((dr = newPos.x - R) < 0) {
                vel.x *= -1;
                return;   
            }
            if ((dr = newPos.x + R - scene->SizeX) > 0) {
                vel.x *= -1;
                return;
            }
            if ((dr = newPos.y + R -scene->SizeY) > 0) {
                vel.y *= -1;
                return;
            }
            if ((dr = newPos.y - R) < 0) {
                scene->resetRaund();
                return;
            }
            if (!(withPlayerCollision(newPos) || withBlocksCollision(newPos)))
                pos = newPos;
        }

        bool    withPlayerCollision (const glm::vec3 &newPos) {
            glm::vec3   p1 = scene->player->pos + 0.5f*scene->player->scl;
            glm::vec3   p2 = scene->player->pos - 0.5f*scene->player->scl;
            p2.y += scene->player->scl.y;

            if (glm::distance(p1, newPos) < R || glm::distance(p2, newPos) < R || (newPos.x < p1.x && newPos.x > p2.x && newPos.y - R < p1.y)) {
                float   angle = M_PI*(pos.x - scene->player->pos.x) / (scene->player->scl.x + R);
                std::cout << angle <<std::endl;
                vel = v_mag*glm::vec3(sin(angle), cos(angle), 0.0f);
                return true;
            }


            return false;
        }

        bool    withBlocksCollision (const glm::vec3 &newPos);

        void reset () {
            pos = scene->player->pos + glm::vec3 (0, R + scene->player->scl.y/2, 0.0);
            vel = glm::vec3 (VelX, VelY, 0.0f);
            runned = false;
        }
};

class   Block : public Object {

    public:

        bool    alive{true};
        Block (glm::vec2 p, Scene *sc, Mesh *m) : Object(sc,m) {
            pos = glm::vec3(p, 0.0f);
            scl = glm::vec3(BlockX, BlockY, 0.0f);
        }

};


Scene::Scene (GLuint sx, GLuint sy, GLuint sh_prog, Mesh **m) : SizeX(sx), SizeY(sy), shaderProgram(sh_prog) {
  
    view = glm::translate (glm::mat4(1.0f), glm::vec3 (-1.f, -1.f, 0.f));
    view = glm::scale (view, glm::vec3 (2.0f/SizeX, 2.0f/SizeY, 1.0f));

    player = new Player (this, m[0]);
    ball = new Ball (this, m[1]);

    float   marginX = (SizeX - 1.1f*BlockX*BlocksCol)/2;
    float   marginY = SizeY - 1.1f*BlockY*BlocksRow/3*4;

    blockMesh = m[0];

    for (int i = 0; i< BlocksRow; i++) {
        float y = marginY + BlockY*1.1f*i;
        for (int j = 0; j < BlocksCol; j++) {
            float x = marginX + BlockX/2 + BlockX*1.1f*j;
            blockList.push_back (*(new Block (glm::vec2 (x, y), this, blockMesh)));
        }
    }
}
void Scene::Draw() {
    player->Draw();
    ball->Draw();

    for (auto &block : blockList)
        block.Draw();
}

void Scene::update(float elapsedTime) {
    player->update(elapsedTime);
    ball->update(elapsedTime);

    if (blockList.size() > 0) {
        auto    i = std::remove_if (blockList.begin(), blockList.end(), [&](Block &o) {return !o.alive;});
        if (i != blockList.end())
            blockList.erase(i);
    } else {
        resetGame();
    }
}

void    Scene::resetRaund () {
    player->reset();
    ball->reset();
}

void    Scene::resetGame () {
    resetRaund ();
    float   marginX = (SizeX - 1.1f*BlockX*BlocksCol)/2;
    float   marginY = SizeY - 1.1f*BlockY*BlocksRow/3*4;

    for (int i = 0; i< BlocksRow; i++) {
        float y = marginY + BlockY*1.1f*i;
        for (int j = 0; j < BlocksCol; j++) {
            float x = marginX + BlockX/2 + BlockX*1.1f*j;
            blockList.push_back (*(new Block (glm::vec2 (x, y), this, blockMesh)));
        }
    }
}

bool    Ball::withBlocksCollision (const glm::vec3 &newPos) {

    for (auto &block : scene->blockList) {
        glm::vec3   p[] = {
            block.pos + 0.5f*block.scl,
            block.pos - 0.5f*block.scl,
            block.pos + 0.5f*block.scl,
            block.pos - 0.5f*block.scl,
        }; 
        p[2].y -= block.scl.y;
        p[3].y += block.scl.y;
        
        for (int i = 0; i < 4; i++) {
            if (glm::distance(p[i], pos) < R) {
                vel *= -1.0f;
                block.alive = false;
                return true;
            }
        }

        if (newPos.x < p[0].x && newPos.x > p[1].x) {
            if ((newPos.y - R < p[0].y && newPos.y - R > p[1].y) ||
                (newPos.y + R < p[0].y && newPos.y + R > p[1].y))  {
                    vel.y *= -1;
                    block.alive = false;
                    return true;
            }
        } else if (newPos.y < p[0].y && newPos.y > p[1].y) {
            if ((newPos.x - R < p[0].x && newPos.x - R > p[1].x) ||
                (newPos.x + R < p[0].x && newPos.x + R > p[1].x))  {
                    vel.x *= -1;
                    block.alive = false;
                    return true;
            }
        }
        
    }

    return false;
}
#endif