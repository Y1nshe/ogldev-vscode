#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "ogldev_math_3d.h"
#include "ogldev_texture.h" // 包含 Texture 类头文件
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include <cstddef> // <-- 添加 cstddef 用于 offsetof

struct Vertex {
    Vector3f pos;
    Vector3f normal; // 添加法线
    Vector2f texCoord; // 新增纹理坐标

    Vertex() {}

    Vertex(Vector3f _pos, Vector3f _normal)
    {
        pos = _pos;
        normal = _normal;
    }

    Vertex(const Vector3f& _pos, const Vector3f& _normal, const Vector2f& _texCoord)
    {
        pos = _pos;
        normal = _normal;
        texCoord = _texCoord;
    }
};

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
#define GRID_SIZE 20

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;
GLuint gWorldLocation;
GLuint gIsWireframeLocation; // 移除不再使用的 uniform 位置变量
GLuint ShaderProgram;
// --- 添加新的 Uniform 位置 ---
GLuint gLightAmbientColorLocation;
GLuint gLightDiffuseColorLocation;
GLuint gLightSpecularColorLocation;
GLuint gLightDirectionLocation;
GLuint gMaterialAmbientColorLocation;
GLuint gMaterialDiffuseColorLocation;
GLuint gMaterialSpecularColorLocation;
GLuint gMaterialShininessLocation;
GLuint gEyeWorldPosLocation;
GLuint gAmbientIntensityLocation;  // 新增: 环境光强度位置
GLuint gDiffuseIntensityLocation; // 新增: 漫反射光强度位置
GLuint gSpecularIntensityLocation; // 新增: 镜面光强度位置
// --------------------------

// --- 输入状态 ---
bool keyStates[256] = {false}; // 跟踪按键状态的数组
bool middleMouseButtonDown = false; // 跟踪鼠标中键状态
int lastMouseX = 0;
int lastMouseY = 0;
// -------------------

// --- 纹理对象 ---
Texture* pTexture1 = NULL;
Texture* pTexture2 = NULL;
Texture* pTexture3 = NULL;
Texture* pTexture4 = NULL;
GLuint gSamplerLocation; // 纹理采样器 uniform 位置
// ------------------

// 相机参数
Vector3f CameraPos(0.0f, 3.0f, 12.0f); // 初始位置
Vector3f CameraUp(0.0f, 1.0f, 0.0f);    // 初始上向量 (实际由 Pitch/Roll 决定)
float cameraYaw = 0.0f;   // 偏航角 (绕Y轴)
float cameraPitch = 0.0f; // 俯仰角 (绕X轴)
float cameraRoll = 0.0f;  // 翻滚角 (绕Z轴)
float CameraSpeed = 0.1f;  // 相机移动速度
float CameraRotateSpeed = 0.2f; // 相机旋转速度 (角度制)
float MouseSensitivity = 0.1f; // 鼠标旋转灵敏度

Camera* pGameCamera = NULL;
PersProjInfo gPersProjInfo;

// --- 光照和材质控制变量 ---
float gAmbientIntensity = 1.0f;
float gDiffuseIntensity = 1.0f;
float gSpecularIntensity = 1.0f;
float gMaterialShininess = 1.0f; // 初始高光系数
const float INTENSITY_STEP = 0.01f; // 光照强度变化步长
const float SHININESS_STEP = 0.1f;  // 高光系数变化步长
const float MIN_SHININESS = -300.0f; // 高光系数最小值
const float MAX_SHININESS = 30.0f;   // 高光系数最大值
// ---------------------------

// --- 辅助函数：渲染文本 ---
void RenderText(float x, float y, void* font, const std::string& text, const Vector3f& color)
{
    GLenum error;
    glUseProgram(0); // 禁用着色器程序以使用固定管线
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "渲染文本前的OpenGL错误: %s\n", gluErrorString(error));
    glDisable(GL_DEPTH_TEST); // 禁用深度测试以确保文本始终可见
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "禁用深度测试后的OpenGL错误: %s\n", gluErrorString(error));
    glColor3f(color.x, color.y, color.z); // 设置文本颜色（固定管线）
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "设置颜色后的OpenGL错误: %s\n", gluErrorString(error));
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT); // 设置2D正交投影
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "设置投影后的OpenGL错误: %s\n", gluErrorString(error));
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); // 设置模型视图矩阵
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "设置模型视图后的OpenGL错误: %s\n", gluErrorString(error));
    glRasterPos2f(x, y); // 设置文本绘制位置
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "设置光栅位置后的OpenGL错误: %s\n", gluErrorString(error));
    for (char c : text) { glutBitmapCharacter(font, c); } // 逐字符渲染
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "渲染字符后的OpenGL错误: %s\n", gluErrorString(error));
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); // 恢复矩阵
    glEnable(GL_DEPTH_TEST); // 重新启用深度测试
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "恢复状态后的OpenGL错误: %s\n", gluErrorString(error));
    glUseProgram(ShaderProgram); // 重新启用着色器程序
    error = glGetError(); if (error != GL_NO_ERROR) fprintf(stderr, "重新启用着色器后的OpenGL错误: %s\n", gluErrorString(error));
}
// --------------------------

// --- 键盘回调函数 ---
static void KeyboardDownCB(unsigned char key, int mouse_x, int mouse_y)
{
    keyStates[key] = true; // 标记按键为按下
}

static void KeyboardUpCB(unsigned char key, int mouse_x, int mouse_y)
{
    keyStates[key] = false; // 标记按键为释放
}
// ---------------------

// --- 鼠标回调函数 ---
static void MouseCB(int button, int state, int x, int y)
{
    if (button == GLUT_MIDDLE_BUTTON) {
        if (state == GLUT_DOWN) {
            middleMouseButtonDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            middleMouseButtonDown = false;
        }
    }
}

static void MotionCB(int x, int y)
{
    if (middleMouseButtonDown) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        cameraYaw   -= (float)deltaX * MouseSensitivity;
        cameraPitch += (float)deltaY * MouseSensitivity;

        // 限制俯仰角
        if (cameraPitch > 89.0f) {
            cameraPitch = 89.0f;
        }
        if (cameraPitch < -89.0f) {
            cameraPitch = -89.0f;
        }

        lastMouseX = x;
        lastMouseY = y;

        // 可选：触发窗口重绘，如果不在 Idle 函数中持续重绘的话
        // glutPostRedisplay();
    }
}
// ----------------------

// --- 根据输入状态更新相机和光照/材质参数 ---
static void UpdateInputState()
{
    // --- 更新光照和材质参数 ---
    if (keyStates['1']) { gAmbientIntensity -= INTENSITY_STEP; if (gAmbientIntensity < 0.0f) gAmbientIntensity = 0.0f; }
    if (keyStates['2']) { gAmbientIntensity += INTENSITY_STEP; }
    if (keyStates['3']) { gDiffuseIntensity -= INTENSITY_STEP; if (gDiffuseIntensity < 0.0f) gDiffuseIntensity = 0.0f; }
    if (keyStates['4']) { gDiffuseIntensity += INTENSITY_STEP; }
    if (keyStates['5']) { gSpecularIntensity -= INTENSITY_STEP; if (gSpecularIntensity < 0.0f) gSpecularIntensity = 0.0f; }
    if (keyStates['6']) { gSpecularIntensity += INTENSITY_STEP; }
    if (keyStates['7']) { gMaterialShininess -= SHININESS_STEP; if (gMaterialShininess < MIN_SHININESS) gMaterialShininess = MIN_SHININESS; }
    if (keyStates['8']) { gMaterialShininess += SHININESS_STEP; if (gMaterialShininess > MAX_SHININESS) gMaterialShininess = MAX_SHININESS; }
    // ---------------------------

    // 计算当前相机方向向量
    const float epsilon = 1e-6f;
    float yawRad = ToRadian(cameraYaw);
    float pitchRad = ToRadian(cameraPitch);
    Vector3f Forward;
    Forward.x = sinf(yawRad) * cosf(pitchRad);
    Forward.y = sinf(pitchRad);
    Forward.z = cosf(yawRad) * cosf(pitchRad);

    // 安全归一化 Forward
    if (Forward.Length() > epsilon) {
        Forward.Normalize();
    } else {
        Forward = Vector3f(0.0f, 0.0f, 1.0f);
        fprintf(stderr, "警告: UpdateInputState 中的 Forward 向量接近零.\n");
    }

    // 安全计算和归一化 Right
    Vector3f Right;
    const Vector3f WorldUp(0.0f, 1.0f, 0.0f);
    if (fabs(Forward.Dot(WorldUp)) > (1.0f - epsilon)) {
        const Vector3f WorldZ(0.0f, 0.0f, 1.0f);
        Right = WorldZ.Cross(Forward);
    } else {
        Right = WorldUp.Cross(Forward);
    }
    if (Right.Length() > epsilon) {
        Right.Normalize();
    } else {
        Right = Vector3f(1.0f, 0.0f, 0.0f);
        fprintf(stderr, "警告: 无法在 UpdateInputState 中计算有效的 Right 向量.\n");
    }

    // 安全计算和归一化 Up
    Vector3f Up = Forward.Cross(Right);
    if (Up.Length() > epsilon) {
        Up.Normalize();
    } else {
        Up = Vector3f(0.0f, 1.0f, 0.0f);
        fprintf(stderr, "警告: 无法在 UpdateInputState 中计算有效的 Up 向量.\n");
    }

    // 检查移动按键
    if (keyStates['w']) { CameraPos -= Forward * CameraSpeed; }
    if (keyStates['s']) { CameraPos += Forward * CameraSpeed; }
    if (keyStates['a']) { CameraPos -= Right * CameraSpeed;   }
    if (keyStates['d']) { CameraPos += Right * CameraSpeed;   }
    if (keyStates['e']) { CameraPos += Up * CameraSpeed;     }
    if (keyStates['c']) { CameraPos -= Up * CameraSpeed;     }

    // 检查旋转按键
    if (keyStates['j']) { cameraYaw += CameraRotateSpeed; }
    if (keyStates['l']) { cameraYaw -= CameraRotateSpeed; }
    if (keyStates['i']) { cameraPitch -= CameraRotateSpeed; if (cameraPitch < -89.0f) cameraPitch = -89.0f; }
    if (keyStates['k']) { cameraPitch += CameraRotateSpeed; if (cameraPitch >  89.0f) cameraPitch =  89.0f; }
}
// ---------------------------

// --- 计算法线的辅助函数 ---
Vector3f CalcNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3) {
    Vector3f edge1 = v2 - v1;
    Vector3f edge2 = v3 - v1;
    Vector3f normal = edge1.Cross(edge2);
    normal.Normalize();
    return normal;
}
// --------------------------

static void RenderSceneCB()
{
    UpdateInputState(); // 首先根据输入状态更新相机和参数

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓冲

    // ---- 动画变量 ----
    static float Scale = 0.0f;      // 控制自转角度 (度)
    // static float OrbitScale = 0.0f; // 控制公转角度 (度) // <-- 注释掉公转角度
    // const float orbitRadius = 5.0f; // 公转半径 // <-- 注释掉公转半径

    Scale += 0.2f; // <-- 降低自转速度
    // OrbitScale += 0.5f; // 公转速度 // <-- 注释掉公转更新
    // -------------------

    // ---- 计算视图矩阵 ----
    const float epsilon_view = 1e-6f; // 视图矩阵计算用 Epsilon
    float yawRad_view = ToRadian(cameraYaw);
    float pitchRad_view = ToRadian(cameraPitch);
    float rollRad_view = ToRadian(cameraRoll);

    Vector3f Forward_view;
    Forward_view.x = sinf(yawRad_view) * cosf(pitchRad_view);
    Forward_view.y = sinf(pitchRad_view);
    Forward_view.z = cosf(yawRad_view) * cosf(pitchRad_view);

    // 归一化 Forward_view
    if (Forward_view.Length() > epsilon_view) {
        Forward_view.Normalize();
    } else {
        Forward_view = Vector3f(0.0f, 0.0f, 1.0f);
        fprintf(stderr, "警告: RenderSceneCB 中的 Forward 向量接近零 (View).\n");
    }

    // 归一化 Right_view
    Vector3f Right_view;
    const Vector3f WorldUp_view(0.0f, 1.0f, 0.0f);
    if (fabs(Forward_view.Dot(WorldUp_view)) > (1.0f - epsilon_view)) {
        const Vector3f WorldZ_view(0.0f, 0.0f, 1.0f);
        Right_view = WorldZ_view.Cross(Forward_view);
    } else {
        Right_view = WorldUp_view.Cross(Forward_view);
    }
    if (Right_view.Length() > epsilon_view) {
        Right_view.Normalize();
    } else {
        Right_view = Vector3f(1.0f, 0.0f, 0.0f);
        fprintf(stderr, "警告: 无法在 RenderSceneCB 中计算有效的 Right 向量 (View).\n");
    }

    // 归一化 Up_view
    Vector3f Up_view = Forward_view.Cross(Right_view);
    if (Up_view.Length() > epsilon_view) {
        Up_view.Normalize();
    } else {
        Up_view = Vector3f(0.0f, 1.0f, 0.0f);
        fprintf(stderr, "警告: 无法在 RenderSceneCB 中计算有效的 Up 向量 (View).\n");
    }

    Matrix4f rollMatrix; float cosR_view = cosf(rollRad_view); float sinR_view = sinf(rollRad_view); Vector3f F_view = Forward_view;
    rollMatrix.m[0][0] = cosR_view + F_view.x * F_view.x * (1 - cosR_view);    rollMatrix.m[0][1] = F_view.x * F_view.y * (1 - cosR_view) - F_view.z * sinR_view; rollMatrix.m[0][2] = F_view.x * F_view.z * (1 - cosR_view) + F_view.y * sinR_view; rollMatrix.m[0][3] = 0.0f;
    rollMatrix.m[1][0] = F_view.y * F_view.x * (1 - cosR_view) + F_view.z * sinR_view; rollMatrix.m[1][1] = cosR_view + F_view.y * F_view.y * (1 - cosR_view);    rollMatrix.m[1][2] = F_view.y * F_view.z * (1 - cosR_view) - F_view.x * sinR_view; rollMatrix.m[1][3] = 0.0f;
    rollMatrix.m[2][0] = F_view.z * F_view.x * (1 - cosR_view) - F_view.y * sinR_view; rollMatrix.m[2][1] = F_view.z * F_view.y * (1 - cosR_view) + F_view.x * sinR_view; rollMatrix.m[2][2] = cosR_view + F_view.z * F_view.z * (1 - cosR_view);    rollMatrix.m[2][3] = 0.0f;
    rollMatrix.m[3][0] = 0.0f;                             rollMatrix.m[3][1] = 0.0f;                             rollMatrix.m[3][2] = 0.0f;                             rollMatrix.m[3][3] = 1.0f;
    Right_view = (rollMatrix * Vector4f(Right_view, 0.0f)).to3f(); // 应用翻滚到 Right
    Up_view    = (rollMatrix * Vector4f(Up_view, 0.0f)).to3f(); // 应用翻滚到 Up

    Vector3f N_view = Forward_view * -1.0f; // 观察方向
    Matrix4f View(Right_view.x, Right_view.y, Right_view.z, -Right_view.Dot(CameraPos),
                  Up_view.x,    Up_view.y,    Up_view.z,    -Up_view.Dot(CameraPos),
                  N_view.x,     N_view.y,     N_view.z,     -N_view.Dot(CameraPos),
                  0.0f,    0.0f,    0.0f,    1.0f);
    // ------------------------------

    // ---- 计算投影矩阵 ----
    float VFOV = 45.0f; float tanHalfVFOV = tanf(ToRadian(VFOV / 2.0f)); float d = 1/tanHalfVFOV;
    float ar = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT; float NearZ = 1.0f; float FarZ = 100.0f;
    float zRange = NearZ - FarZ; float A = (-FarZ - NearZ) / zRange; float B = 2.0f * FarZ * NearZ / zRange;
    Matrix4f Projection(d/ar, 0.0f, 0.0f, 0.0f, 0.0f, d, 0.0f, 0.0f, 0.0f, 0.0f, A, B, 0.0f, 0.0f, 1.0f, 0.0f);
    // ----------------------

    // ---- 计算物体模型矩阵 (仅自转) ----
    Matrix4f SelfRotationMatrix; SelfRotationMatrix.InitRotateTransform(0.0f, Scale, 0.0f); // 绕 Y 轴自转 (度)
    // Matrix4f OrbitTranslationMatrix; OrbitTranslationMatrix.InitTranslationTransform(orbitRadius, 0.0f, 0.0f); // 沿 X 轴移出 // <-- 注释掉公转平移
    // Matrix4f OrbitRotationMatrix; OrbitRotationMatrix.InitRotateTransform(0.0f, OrbitScale, 0.0f); // 绕世界 Y 轴公转 (度) // <-- 注释掉公转旋转
    Matrix4f Model = SelfRotationMatrix; // <-- 修改：模型矩阵只包含自转
    // ------------------------------------------

    // ---- 设置 World 矩阵 ---
    const Matrix4f& World = Model;
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)&World);
    // -----------------------

    // --- 设置光照和材质 Uniforms ---
    glUniform3f(gLightAmbientColorLocation, 1.0f, 1.0f, 1.0f);
    glUniform3f(gLightDiffuseColorLocation, 1.0f, 0.1f, 0.5f);
    glUniform3f(gLightSpecularColorLocation, 0.0f, 1.0f, 0.0f);
    Vector3f LightDirection = Vector3f(1.0f, 1.0f, 0.0f);
    LightDirection.Normalize();
    glUniform3fv(gLightDirectionLocation, 1, (const GLfloat*)&LightDirection);

    glUniform3f(gMaterialAmbientColorLocation, 1.0f, 1.0f, 1.0f);
    glUniform3f(gMaterialDiffuseColorLocation, 1.0f, 0.0f, 0.0f);
    glUniform3f(gMaterialSpecularColorLocation, 0.0f, 1.0f, 1.0f);
    glUniform1f(gMaterialShininessLocation, gMaterialShininess);
    glUniform3fv(gEyeWorldPosLocation, 1, (const GLfloat*)&CameraPos);
    // --- 更新光照强度和高光系数 Uniform ---
    glUniform1f(gAmbientIntensityLocation, gAmbientIntensity);
    glUniform1f(gDiffuseIntensityLocation, gDiffuseIntensity);
    glUniform1f(gSpecularIntensityLocation, gSpecularIntensity);
    // -----------------------------------------

    // ---- 渲染三棱锥 ----
    glUseProgram(ShaderProgram);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0); // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1); // 法线
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, normal)); // 使用 offsetof
    glEnableVertexAttribArray(2); // <-- 新增: 启用纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, texCoord)); // <-- 新增: 设置纹理坐标指针

    Matrix4f FinalMatrix = Projection * View * Model; // 计算最终变换矩阵
    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, &FinalMatrix.m[0][0]); // 发送矩阵到着色器

    // 第一次绘制：填充面
    // glUniform1i(gIsWireframeLocation, 0); // 移除设置 isWireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 设置多边形填充模式
    glUniform1i(gSamplerLocation, 0); // 绑定纹理单元 0

    // 绘制面 1 (纹理 1)
    pTexture1->Bind(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 绘制面 2 (纹理 2)
    pTexture2->Bind(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 3, 3);

    // 绘制面 3 (纹理 3)
    pTexture3->Bind(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 6, 3);

    // 绘制面 4 (底面) (纹理 4)
    pTexture4->Bind(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 9, 3);

    // 恢复状态
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2); // <-- 新增: 禁用纹理坐标
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 恢复多边形填充模式
    // ------------------

    // ---- 渲染调试文本 ----
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Cam Pos: (" << CameraPos.x << ", " << CameraPos.y << ", " << CameraPos.z << ")";
    RenderText(10.0f, WINDOW_HEIGHT - 20.0f, GLUT_BITMAP_HELVETICA_18, oss.str(), Vector3f(1.0f, 1.0f, 0.0f));
    oss.str(""); oss.clear();
    oss << "Cam Rot: (" << cameraRoll << ", " << cameraYaw << ", " << cameraPitch << ")";
    RenderText(10.0f, WINDOW_HEIGHT - 40.0f, GLUT_BITMAP_HELVETICA_18, oss.str(), Vector3f(1.0f, 1.0f, 0.0f));
    oss.str(""); oss.clear();
    oss << "Obj Pos: (" << Model.m[0][3] << ", " << Model.m[1][3] << ", " << Model.m[2][3] << ")";
    RenderText(10.0f, WINDOW_HEIGHT - 60.0f, GLUT_BITMAP_HELVETICA_18, oss.str(), Vector3f(1.0f, 1.0f, 0.0f));
    oss.str(""); oss.clear();
    oss << "Obj Anim: (" << fmodf(Scale, 360.0f) << ")";
    RenderText(10.0f, WINDOW_HEIGHT - 80.0f, GLUT_BITMAP_HELVETICA_18, oss.str(), Vector3f(1.0f, 1.0f, 0.0f));
    oss.str(""); oss.clear();
    oss << "Ambient: " << gAmbientIntensity << " (1/2) Diffuse: " << gDiffuseIntensity << " (3/4) Specular: " << gSpecularIntensity << " (5/6) Shininess: " << gMaterialShininess << " (7/8)";
    RenderText(10.0f, WINDOW_HEIGHT - 100.0f, GLUT_BITMAP_HELVETICA_18, oss.str(), Vector3f(1.0f, 1.0f, 1.0f));
    // ----------------------

    glutPostRedisplay(); // 请求重绘下一帧
    glutSwapBuffers(); // 交换前后缓冲
}

static void CreateVertexBuffer()
{
    // 定义四面体的 4 个基础顶点位置
    Vector3f Top(0.0f, 1.0f, 0.0f);
    Vector3f Base1(1.0f, -1.0f, 1.0f);  // 右前
    Vector3f Base2(-1.0f, -1.0f, 1.0f); // 左前
    Vector3f Base3(0.0f, -1.0f, -1.0f); // 后

    // 计算每个面的法线 (确保顶点顺序符合右手定则以得到朝外的法线)
    Vector3f frontNormal  = CalcNormal(Top, Base2, Base1);  // 前: Top-Left-Right
    Vector3f leftNormal   = CalcNormal(Top, Base3, Base2);  // 左: Top-Back-Left
    Vector3f rightNormal  = CalcNormal(Top, Base1, Base3);  // 右: Top-Right-Back
    Vector3f bottomNormal = CalcNormal(Base1, Base2, Base3); // 底: Right-Left-Back (注意顶点顺序)

    // 定义纹理坐标 (参照 Experiment02，中心点在顶部)
    Vector2f uvTop(0.5f, 1.0f);   // 顶部中心
    Vector2f uvBL(0.0f, 0.0f);    // 左下角
    Vector2f uvBR(1.0f, 0.0f);    // 右下角
    Vector2f uvBC(0.5f, 0.0f);    // 底部中心 (可能用于底面?)

    // 定义 12 个顶点，每个面 3 个，包含位置、法线和纹理坐标
    Vertex Vertices[12] = {
        // 面 1: Top, Base2, Base1 (前) - 法线 frontNormal
        Vertex(Top,   frontNormal, uvTop), // 顶部
        Vertex(Base2, frontNormal, uvBL),  // 左下
        Vertex(Base1, frontNormal, uvBR),  // 右下

        // 面 2: Top, Base3, Base2 (左) - 法线 leftNormal
        Vertex(Top,   leftNormal, uvTop), // 顶部
        Vertex(Base3, leftNormal, uvBL),  // 左下 (对应面的左下)
        Vertex(Base2, leftNormal, uvBR),  // 右下 (对应面的右下)

        // 面 3: Top, Base1, Base3 (右) - 法线 rightNormal
        Vertex(Top,   rightNormal, uvTop), // 顶部
        Vertex(Base1, rightNormal, uvBL),  // 左下 (对应面的左下)
        Vertex(Base3, rightNormal, uvBR),  // 右下 (对应面的右下)

        // 面 4: Base1, Base2, Base3 (底) - 法线 bottomNormal
        // 底面的纹理映射可能需要调整，这里用 Base1=BR, Base2=BL, Base3=Top? 需测试
        Vertex(Base1, bottomNormal, uvBR), // 右前角 -> 纹理右下
        Vertex(Base2, bottomNormal, uvBL), // 左前角 -> 纹理左下
        Vertex(Base3, bottomNormal, uvTop) // 后角 -> 纹理顶部中心 (或者 uvBC?)
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
    unsigned int Indices[] = {
        // 注意：为了正确光照，可能需要调整顶点顺序或法线方向
        // 确保法线指向外部 (符合右手定则)
        0, 1, 2, // Front face (adjust winding if needed)
        0, 3, 1, // Left face (adjust winding if needed)
        0, 2, 3, // Right face (adjust winding if needed)
        1, 3, 2  // Bottom face (adjust winding if needed)
    };

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(1);
    }

    const GLchar* p[1];
    p[0] = pShaderText;

    GLint Lengths[1];
    Lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

const char* pVSFileName = "lighting_shader.vs";
const char* pFSFileName = "lighting_shader.fs";

static void CompileShaders()
{
    ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(pVSFileName, vs)) {
        exit(1);
    };

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(pFSFileName, fs)) {
        exit(1);
    };

    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    if (gWorldLocation == -1) {
        printf("Error getting uniform location of 'gWorld'\n");
        exit(1);
    }

    gSamplerLocation = glGetUniformLocation(ShaderProgram, "gSampler"); // <-- 获取 gSampler 位置
    if (gSamplerLocation == -1) {
        printf("Error getting uniform location of 'gSampler'\n");
        exit(1);
    }

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);

    gEyeWorldPosLocation = glGetUniformLocation(ShaderProgram, "gEyeWorldPos");
    assert(gEyeWorldPosLocation != 0xFFFFFFFF);

    gLightAmbientColorLocation = glGetUniformLocation(ShaderProgram, "gLightAmbientColor");
    assert(gLightAmbientColorLocation != 0xFFFFFFFF);
    gLightDiffuseColorLocation = glGetUniformLocation(ShaderProgram, "gLightDiffuseColor"); // <-- 恢复获取漫反射光位置
    assert(gLightDiffuseColorLocation != 0xFFFFFFFF); // <-- 恢复断言
    gLightSpecularColorLocation = glGetUniformLocation(ShaderProgram, "gLightSpecularColor");
    assert(gLightSpecularColorLocation != 0xFFFFFFFF);
    gLightDirectionLocation = glGetUniformLocation(ShaderProgram, "gLightDirection");
    assert(gLightDirectionLocation != 0xFFFFFFFF);

    gMaterialAmbientColorLocation = glGetUniformLocation(ShaderProgram, "gMaterialAmbientColor");
    assert(gMaterialAmbientColorLocation != 0xFFFFFFFF);
    // gMaterialDiffuseColorLocation = glGetUniformLocation(ShaderProgram, "gMaterialDiffuseColor"); // <-- 移除获取材质漫反射颜色位置
    gMaterialSpecularColorLocation = glGetUniformLocation(ShaderProgram, "gMaterialSpecularColor");
    assert(gMaterialSpecularColorLocation != 0xFFFFFFFF);
    gMaterialShininessLocation = glGetUniformLocation(ShaderProgram, "gMaterialShininess");
    assert(gMaterialShininessLocation != 0xFFFFFFFF);

    // --- 获取新增的 Uniform 位置 ---
    gAmbientIntensityLocation = glGetUniformLocation(ShaderProgram, "gAmbientIntensity");
    gDiffuseIntensityLocation = glGetUniformLocation(ShaderProgram, "gDiffuseIntensity");
    gSpecularIntensityLocation = glGetUniformLocation(ShaderProgram, "gSpecularIntensity");
    // ------------------------------

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    int x = 200; int y = 100; glutInitWindowPosition(x, y);
    int win = glutCreateWindow("实验 03"); // 修改窗口标题
    printf("窗口 ID: %d\n", win);
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "错误: '%s'\n", glewGetErrorString(err));
        return 1;
    }

    printf("GL 版本: %s\n", glGetString(GL_VERSION));

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f); // 设置背景色为深灰色
    glEnable(GL_DEPTH_TEST); // 启用深度测试

    // --- 加载纹理 ---
    pTexture1 = new Texture(GL_TEXTURE_2D, "../Content/Experiments/face01.png");
    if (!pTexture1->Load()) { fprintf(stderr, "Error loading texture face01.png\n"); return 1; }
    pTexture2 = new Texture(GL_TEXTURE_2D, "../Content/Experiments/face02.png");
    if (!pTexture2->Load()) { fprintf(stderr, "Error loading texture face02.png\n"); return 1; }
    pTexture3 = new Texture(GL_TEXTURE_2D, "../Content/Experiments/face03.png");
    if (!pTexture3->Load()) { fprintf(stderr, "Error loading texture face03.png\n"); return 1; }
    pTexture4 = new Texture(GL_TEXTURE_2D, "../Content/Experiments/face04.png");
    if (!pTexture4->Load()) { fprintf(stderr, "Error loading texture face04.png\n"); return 1; }
    // ------------------

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB); // 使用 Idle 函数实现连续渲染和输入处理
    glutKeyboardFunc(KeyboardDownCB);
    glutKeyboardUpFunc(KeyboardUpCB);
    glutMouseFunc(MouseCB);       // 注册鼠标按钮回调
    glutMotionFunc(MotionCB);     // 注册鼠标移动回调 (按钮按下时)

    glutMainLoop();

    // --- 释放纹理资源 (虽然程序结束会自动释放，但良好习惯是手动释放) ---
    delete pTexture1;
    delete pTexture2;
    delete pTexture3;
    delete pTexture4;
    // ------------------------------------------------------------------

    return 0;
}
