#ifndef     OBJECTS_H
#define     OBJECTS_H


#include    <GL/glew.h>
#include    <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include    <iostream>
#include    <vector>
#include    <cmath>
#include    <algorithm>

class   Block;
class   Player;
class   Ball;


struct  Scene {

    GLuint  SizeX, SizeY;
    GLuint  BlocksRow, BlocksCol;

    Player  *player{nullptr};
    Ball    *ball{nullptr};

    GLuint      shaderProgram;
    glm::mat4   view;

    Scene (GLuint sx, GLuint sy, GLuint sh_prog) : SizeX(sx), SizeY(sy), shaderProgram(sh_prog) {
        BlocksRow = 7;
        BlocksCol = 15; 

        view = glm::translate (glm::mat4(1.0f), glm::vec3 (-1.f, -1.f, 0.f));
        view = glm::scale (view, glm::vec3 (2.0f/SizeX, 2.0f/SizeY, 1.0f));
    }
};

class   Object {
    public:
        Scene   *scene{nullptr};
};

class   PhisicObject : public Object {

    public:

        glm::vec3   pos;
        glm::vec3   scl;

        virtual void update (float elapsedTime) {

        }
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


struct   Perfab : public Object {

    PhisicObject    *obj{nullptr};
    Mesh            *mesh{nullptr};

    Perfab (PhisicObject *o, Mesh *m) : obj(o), mesh(m) {
        scene = (obj->scene != nullptr) ? obj->scene : nullptr;
        std::cout << scene << std::endl;
    }

    void    Draw () {
        glBindVertexArray (mesh->VAO);

        glm::mat4   model = glm::translate (glm::mat4 (1.0f), obj->pos);
        model = glm::scale (model, obj->scl);

        GLuint      viewLoc = glGetUniformLocation (scene->shaderProgram, "view");
        glUniformMatrix4fv (viewLoc, 1, GL_FALSE, glm::value_ptr(scene->view));

        GLuint     modelLoc = glGetUniformLocation (scene->shaderProgram, "model");
        glUniformMatrix4fv (modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        mesh->Draw();

        glBindVertexArray (0);
    }
};


class   Block : public PhisicObject {
    
    public:
        Block (glm::vec2 p, Scene *sc) {
            scene = sc;
            pos = glm::vec3 (p, 0.0f);
            scl = glm::vec3 (3.0f, 1.0f, 0.0f);
        }
        ~Block () {}

        void update (float elaepsedTime) {
            alive = !ballCollision ();
        }

        bool ballCollision ();

        bool    alive{true};
};

bool    keys[1024];

class   Player : public PhisicObject {

    public:
        Player (Scene *sc) {
            scene = sc;
            scl = glm::vec3 (14.5f, 1.0f, 1.0f);
            pos = glm::vec3 (scene->SizeX/2.0f, scl.y, 0.0f);
            scene->player = this;
        }
        ~Player () {}

        float   v_mag{100.f};

        void update (float elapsedTime) override {
            move (elapsedTime);
            ballCollision();
        }

        void ballCollision ();

        void move (float elapsedTime) {
            float xPos = pos.x;

            if (keys[GLFW_KEY_LEFT]) {
                xPos -= v_mag*elapsedTime;
            }                 
            if (keys[GLFW_KEY_RIGHT]) {
                xPos += v_mag*elapsedTime;
            }

            if (xPos - scl.x/2 > 0 && xPos + scl.x/2 < scene->SizeX) {
                pos.x = xPos;
            }
        }
};

class   Ball : public PhisicObject{

    public:
        Ball (Scene *sc)  {
            scene = sc;
            GLfloat yPos = scene->player->pos.y + scene->player->scl.y/2 + radius;
            pos = glm::vec3 ((GLfloat)scene->SizeX/2, yPos, 0.0f);
            scl = glm::vec3 (2*radius, 2*radius, 1.0f);
            scene->ball = this;
        }
        ~Ball () {}

        glm::vec3   velocity{glm::vec3(0.f,0.f,0.f)};     
        GLfloat     radius{1.f};          
        bool        runned{false};
        float       v_mag{400.f};
        float       v_angle{M_PI_4};

        void update (float elapsedTime) {
            if (!runned) {
                if (keys[GLFW_KEY_SPACE]) {
                    runned = true;
                    velocity = glm::vec3 (v_mag*cos(v_angle), v_mag*sin(v_angle), 0.f);
                } 
                pos.x = scene->player->pos.x;
            } else {
                move(elapsedTime);
            }
        }

        void move (float elapsedTime) {
            glm::vec3 tmp_pos =  pos + velocity*elapsedTime;

            bool    bumped = false; 
            float   dr;
            if ((dr = tmp_pos.x-radius) < 0) {
                velocity.x *= -1;
                bumped = true;
            } else if ((dr = tmp_pos.x + radius - scene->SizeX) > 0 ) {
                velocity.x *= -1;
                bumped = true;
            }

            if ((dr = tmp_pos.y + radius - scene->SizeY) > 0 ) {
                velocity.y *= -1;
                bumped = true;
            } else if ((dr = tmp_pos.y - radius) < 0) {
                // scene->player->pos.x = scene->SizeX/2;
                // tmp_pos.x = scene->SizeX/2;
                // tmp_pos.y = scene->player->pos.y + scene->player->scl.y/2 + radius;
                // runned = false;
                // bumped = true;
                velocity.y *= -1;
                bumped = true;
            }

            if (!bumped) pos = tmp_pos;
        }


};

void Player::ballCollision () {
    glm::vec3   dr = scene->ball->pos - pos;
    float tmp_y = dr.y - (scene->ball->radius + scl.y/2);
    if (dr.x < scl.x/2 && dr.x > -scl.x/2 && tmp_y < 0) {
        float   angle = 2*dr.x/scl.x;
        scene->ball->velocity = scene->ball->v_mag * glm::vec3 (sin(angle), cos(angle), 0.0f);
        // scene->ball->velocity = scene->ball->v_mag * glm::vec3 (0.0f, 1.0f, 0.0f);
        scene->ball->pos.y -= tmp_y;
    }
}
bool Block::ballCollision () {
    Ball        *ball = scene->ball;

    glm::vec3   p = glm::vec3(pos.x+scl.x/2, pos.y+scl.y/2, 0.0f);
    if (glm::distance(p, ball->pos) < ball->radius) {
        ball->velocity *= -1;
        ball->pos += ball->pos - p;
        return true;
    }
    p = glm::vec3(pos.x+scl.x/2, pos.y-scl.y/2, 0.0f);
    if (glm::distance(p, ball->pos) < ball->radius) {
        ball->velocity *= -1;
        ball->pos += ball->pos - p;
        return true;
    }
    p = glm::vec3(pos.x-scl.x/2, pos.y+scl.y/2, 0.0f);
    if (glm::distance(p, ball->pos) < ball->radius) {
        ball->velocity *= -1;
        ball->pos += ball->pos - p;
        return true;
    }
    p = glm::vec3(pos.x-scl.x/2, pos.y-scl.y/2, 0.0f);
    if (glm::distance(p, ball->pos) < ball->radius) {
        ball->velocity *= -1;
        ball->pos += ball->pos - p;
        return true;
    }

    p = ball->pos;
    if (p.x > pos.x-scl.x/2 && p.x < pos.x+scl.x/2) {
        if (p.y+ball->radius > pos.y-scl.y/2 && p.y+ball->radius < pos.y+scl.y/2) {
            ball->pos.y -= ball->pos.y + ball->radius - (pos.y-scl.y/2);
            ball->velocity.y *= -1;
            return true;
        }
        if (p.y-ball->radius > pos.y-scl.y/2 && p.y-ball->radius < pos.y+scl.y/2) {
            ball->pos.y += ball->pos.y + ball->radius - (pos.y+scl.y/2);
            ball->velocity.y *= -1;
            return true;
        }
    }
    if (p.y > pos.y-scl.y/2 && p.y < pos.y+scl.y/2) {
        if (p.x+ball->radius > pos.x-scl.x/2 && p.x+ball->radius < pos.x+scl.x/2) {
            ball->pos.x -= ball->pos.x + ball->radius - (pos.x-scl.x/2);
            ball->velocity.x *= -1;
            return true;
        }
        if (p.x-ball->radius > pos.x-scl.x/2 && p.x-ball->radius < pos.x+scl.x/2) {
            ball->pos.x += ball->pos.x + ball->radius - (pos.x+scl.x/2);
            ball->velocity.x *= -1;
            return true;
        }

        return false;
    }
}

#endif