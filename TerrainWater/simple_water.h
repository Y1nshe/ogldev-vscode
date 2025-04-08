/*
    Copyright 2022 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SIMPLE_WATER_H
#define SIMPLE_WATER_H

#include "ogldev_texture.h"
#include "ogldev_framebuffer.h"
#include "simple_water_technique.h"
#include "triangle_list.h"

class SimpleWater {
 public:

    SimpleWater();

    ~SimpleWater();

    void Init(int Size, float WorldScale);

    void SetWaterHeight(float Height) { m_waterHeight = Height; }

    float GetWaterHeight() const { return m_waterHeight; }

    void Render(const Vector3f& CameraPos, const Matrix4f& VP, const Vector3f& LightDir);

    GLuint GetDUDVTexture() { return m_dudvMap.GetTexture(); }

    void StartReflectionPass();
    void EndReflectionPass();

    void StartRefractionPass();
    void EndRefractionPass();

    GLuint GetReflectionTexture() const { return m_reflectionFBO.GetTexture(); }

    GLuint GetRefractionTexture() const { return m_refractionFBO.GetTexture(); }

 private:
    TriangleList m_water;
    SimpleWaterTechnique m_waterTech;
    float m_waterHeight = 0.0f;
    Framebuffer m_reflectionFBO;
    Framebuffer m_refractionFBO;
    Texture m_dudvMap;
    Texture m_normalMap;
};

#endif
