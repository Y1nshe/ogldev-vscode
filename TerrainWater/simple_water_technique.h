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

#ifndef SIMPLE_WATER_TECHNIQUE_H
#define SIMPLE_WATER_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

class SimpleWaterTechnique : public Technique
{
public:

    SimpleWaterTechnique();

    virtual bool Init();

    void SetVP(const Matrix4f& VP);
    void SetReflectionTextureUnit(unsigned int TextureUnit);
    void SetRefractionTextureUnit(unsigned int TextureUnit);
    void SetDUDVMapTextureUnit(unsigned int TextureUnit);
    void SetNormalMapTextureUnit(unsigned int TextureUnit);
    void SetDepthMapTextureUnit(unsigned int TextureUnit);
    void SetWaterHeight(float Height);
    void SetDUDVOffset(float Offset);
    void SetCameraPos(const Vector3f& CameraPos);
    void SetLightColor(const Vector3f& LightColor);
    void SetLightDir(const Vector3f& LightDir);

private:
    GLuint m_VPLoc = -1;
    GLuint m_reflectionTexUnitLoc = -1;
    GLuint m_refractionTexUnitLoc = -1;
    GLuint m_heightLoc = -1;
    GLuint m_dudvMapTexUnitLoc = -1;
    GLuint m_normalMapTexUnitLoc = -1;
    GLuint m_depthMapTexUnitLoc = -1;
    GLuint m_dudvOffsetLoc = -1;
    GLuint m_cameraPosLoc = -1;    
    GLuint m_lightColorLoc = -1;
    GLuint m_reversedLightDirLoc = -1;
};

#endif  /* SIMPLE_WATER_TECHNIQUE_H */
