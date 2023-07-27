//+--------------------------------------------------------------------------
//
// File:        drawing.h
//
// NightDriverStrip - (c) 2018 Plummer's Software LLC.  All Rights Reserved.  
//
// This file is part of the NightDriver software project.
//
//    NightDriver is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//   
//    NightDriver is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//   
//    You should have received a copy of the GNU General Public License
//    along with Nightdriver.  It is normally found in copying.txt
//    If not, see <https://www.gnu.org/licenses/>.
//
// Description:
//
//    Functions exported from Acceleration.cpp
//
// History:     Nov-07-2022         IanEl      Created
//
//---------------------------------------------------------------------------

//void DrawPlaceholderDisplay();

#pragma once

void IRAM_ATTR AccelLoopTaskEntry(void *);

class accelData
{

private:
    float xPos_;
    float xNeg_;
    float y_;
    float z_;
    float avXPos_;
    float avXNeg_;

    float gyrX_;
    float gyrY_;
    float gyrZ_;

public:
    //read only with function to set.
    float xPos() const { return xPos_; }
    void SetXPos(float xPos)
    {
        xPos_ = xPos;
    }

    float xNeg() const { return xNeg_; }
    void SetXNeg(float xNeg)
    {
        xNeg_ = xNeg;
    }
    
    float y() const { return z_; }
    void SetY(float y)
    {
        y_ = y;
    }
    
    float z() const { return z_; }
    void SetZ(float z)
    {
        z_ = z;
    }
    
    float avXPos() const { return avXPos_; }
    void SetAvXPos(float avXPos)
    {
        avXPos_ = avXPos;
    }

    float avXNeg() const { return avXNeg_; }
    void SetAvXNeg(float avXNeg)
    {
        avXNeg_ = avXNeg;
    }

    float GyrX() const { return gyrX_; }
    void SetGyrX(float x)
    {
        gyrX_ = x;
    }
    float gyrY() const { return gyrY_; }
    void SetGyrY(float y)
    {
        gyrY_ = y;
    }
    float gyrZ() const { return gyrZ_; }
    void SetGyrZ(float z)
    {
        gyrZ_ = z;
    }
};