#include "globals.h"
#include "effects/effectmanager.h"
#include "ledbuffer.h"
#include "ntptimeclient.h"
#include "remotecontrol.h"
#include <mutex>
#include <ArduinoOTA.h>             // Over-the-air helper object so we can be flashed via WiFi
#include "ntptimeclient.h"
#include "acceleration.h"

extern accelData g_accel;

void IRAM_ATTR AccelLoopTaskEntry(void *)
{
    debugI(">> AccelLoopTaskEntry\n");
    debugE("Entry Heap: %s", heap_caps_check_integrity_all(true) ? "PASS" : "FAIL");
    
    // This thread should not return
    for (;;)
    {
        static float rollingGUAv;
        static float rollingGUAvPos;
        static float rollingGUAvNeg;
        static float accAlpha = ROLLING_AVERAGE_ALPHA;

        static float debugCounter;
        
        // linear
        float accX = 0;
        float accY = 0;
        float accZ = 0;

        // rotational
        float gyrX = 0;
        float gyrY = 0;
        float gyrZ = 0;

        //grabbing raw values from IMU
        M5.Imu.getAccelData(&accX, &accY, &accZ);
        M5.Imu.getGyroData(&gyrX, &gyrY, &gyrZ);
        // g_accel.SetX(accX); doing something funky with X
        g_accel.SetY(accY);
        g_accel.SetZ(accZ);
        g_accel.SetGyrX(gyrX);
        g_accel.SetGyrY(gyrY);
        g_accel.SetGyrZ(gyrZ);
        
        // used to determine orientation for purposes of removing acceleration due to gravity. SUPER HACKY
        rollingGUAv = (accAlpha * accX) + (1.0 - accAlpha) * rollingGUAv;

        if(rollingGUAv > 0)
        {
            accX -= 0.98f;
            g_accel.SetXPos(accX);
        }
        if(rollingGUAv < 0)
        {
            accX += 0.98f;
            g_accel.SetXNeg(accX);
        }

        if(accX > 0)
        {
            rollingGUAvPos = (accAlpha  * accX) + (1.0 - accAlpha ) * rollingGUAvPos;
            g_accel.SetAvXPos(rollingGUAvPos);
            rollingGUAvNeg *= 0.9f;
        }
        if(accX < 0)
        {
            rollingGUAvNeg = (accAlpha  * abs(accX)) + (1.0 - accAlpha ) * rollingGUAvNeg;
            g_accel.SetAvXNeg(rollingGUAvNeg);
            rollingGUAvPos *= 0.9f;
        }
        
        //debugI("Xa: %8.2f, Ya: %8.2f Za: %8.2f", g_accel.x(),accY,accZ);
        
        if(debugCounter > 10)
        {
            //debugI("Xacc: %8.2f: ", rollingGUAvPos);
            //debugI("Xacc: %8.2f XavPos: %8.2f XavPNeg: -%8.2f ",accX, rollingGUAvPos, rollingGUAvNeg );
            debugI("Xg: %8.2f, Yg: %8.2f Zg: %8.2f", gyrX,gyrY,gyrZ);
            debugCounter = 0;
        }
        
        // Update decorum. No need for this to run much while updates happen.
        if (g_bUpdateStarted)
            delay(1000);

        //purely  debug tool. BUGBUG
        debugCounter++;

        //IMPORTANT or this never yields to other threads and they fail
        delay(25);
    }
    
}