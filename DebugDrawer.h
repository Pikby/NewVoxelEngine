#pragma once
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBox2dShape.h>


#include <glad/glad.h> 
#include <GLFW/glfw3.h>
class DebugDrawer : public btIDebugDraw {
    unsigned int VAO, VBO;
    int debugMode = 1;
    std::vector<glm::vec3> lines;
public:
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)override {
        // Vertex data
        GLfloat points[6];

        lines.push_back({ from.x(),from.y(),from.z() });
        lines.push_back({ to.x(),to.y(),to.z() });





    }

    void draw(Shader& shader) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        shader.use();


        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, lines.size()*sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, lines.size());
        glBindVertexArray(0);

        lines.clear();
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) {
        drawLine(from, to, fromColor);
    }

    void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) {
        //justdont
    }

    void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) {
        drawLine(a, b, color);
        drawLine(b, c, color);
        drawLine(c, a, color);
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
        //justdont
    }

    void reportErrorWarning(const char* warningString) {
        std::cout << "PHYSICS ENGINE ERROR: " << warningString << "\n";
    }

    void draw3dText(const btVector3& location, const char* textString) {
        //why the fuck is this a thing
    }

    void setDebugMode(int DebugMode) {
        debugMode = DebugMode;
    }

    virtual int     getDebugMode() const { return debugMode; }
};