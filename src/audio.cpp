//+--------------------------------------------------------------------------
//
// File:        audio.cpp
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
//
// Description:
//
//    Source files for NightDriverStrip's audio processing
//
// History:     Apr-13-2019         Davepl      Created for NightDriverStrip
//              
//---------------------------------------------------------------------------

#include "globals.h"                      // CONFIG and global headers
#if ENABLE_AUDIO
#include "soundanalyzer.h"

extern DRAM_ATTR uint32_t g_FPS;          // Our global framerate

float SampleBuffer::_oldVU;
float SampleBuffer::_oldPeakVU;
float SampleBuffer::_oldMinVU;

float PeakData::_Min[NUM_BANDS] = { 0.0 };
float PeakData::_Max[NUM_BANDS] = { 0.0 }; 
float PeakData::_Last[NUM_BANDS] = { 0.0 }; 
float PeakData::_allBandsMax = 1.0;

// BUGBUG (Davepl) - Time to collect all of these into an Audio class, I'd say!

float gScaler = 0.0f;					  // Instantaneous read of LED display vertical scaling
float gLogScale = 1.0f;					  // How exponential the peaks are made to be
volatile float gVURatio = 1.0;		  		  // Current VU as a ratio to its recent min and max
volatile float gVU = 0;					  // Instantaneous read of VU value
volatile float gPeakVU = MAX_VU;		  	  // How high our peak VU scale is in live mode
volatile float gMinVU = 0;		        	  // How low our peak VU scale is in live mode
volatile unsigned long g_cSamples = 0;			  // Total number of samples successfully collected
int g_AudioFPS = 0;				          // Framerate of the audio sampler
unsigned long g_lastPeak1Time[NUM_BANDS] = { 0 } ;
float g_peak1Decay[NUM_BANDS] = { 0 };
float g_peak2Decay[NUM_BANDS] = { 0 };
float g_peak1DecayRate;
float g_peak2DecayRate;

// Depending on how many bands have been defined, one of these tables will contain the frequency
// cutoffs for that "size" of a spectrum display.  

#if NUM_BANDS == 16
	const int cutOffsBand[16] =
	{
		350,860,1370,1880,2390,2900,3410,3920,4430,4940,5450,5960,6470,6980,7490,8000
	};
	#if TTGO	
		const float scalarsBand[16] = 
		{
			0.05f, 0.15f, 0.2f, 0.225f, 0.25f, 0.3f, 0.35f, 0.4f, 0.425f, 0.6f, 0.7f, 0.8f, 0.8f, 0.9f, 1.0f, 1.0f
		};
	#else
		const float scalarsBand[16] = 
		{
			0.4f, 1.0f, 1.0f, 1.0f, 0.8f, 1.2f, 1.2f, 1.5, 1.5f, 1.4f, 1.2f, 0.8f, 0.8f, 1.0f, 10.0f, 0.8f
		};
	#endif
#endif

#if NUM_BANDS == 10
	const int cutOffsBand[10] =
	{
		250, 550, 1000, 1500, 2250, 3000, 4000, 5500, 7500, 10000
	};
	#if TTGO	
		const float scalarsBand[16] = 
		{
			0.05f, 0.15f, 0.2f, 0.225f, 0.25f, 0.3f, 0.35f, 0.4f, 0.425f, 0.6f, 0.7f, 0.8f, 0.8f, 0.9f, 1.0f, 1.0f
		};
	#else
	// gain for each band
		const float scalarsBand[10] = 
		{
			0.1f, 0.2f, 0.3f, 0.5f, 0.6f, 0.7f, 0.75f, 0.6f, 0.6f, 0.6f
		};
	#endif
#endif

#if NUM_BANDS == 12
	const int cutOffsBand[12] =
	{
		250, 550, 1000, 1500, 2250, 3000, 3500, 4000, 5000, 5500, 7500, 10000
	};
	#if TTGO	
		const float scalarsBand[12] = 
		{
			0.05f, 0.15f, 0.2f, 0.225f, 0.25f, 0.3f, 0.35f, 0.4f, 0.425f, 0.6f, 0.7f, 0.8f, 0.8f, 0.9f, 1.0f, 1.0f
		};
	#else
	// gain for each band
		const float scalarsBand[12] = 
		{
			0.2f, 0.2f, 0.25f, 0.3f, 0.5f, 0.6f, 0.7f, 0.75f, 0.6f, 0.6f, 0.7f, 0.7f
		};
	#endif
#endif

#if NUM_BANDS == 13
	const int cutOffsBand[13] =
	{
		250, 550, 1000, 1500, 2250, 3000, 3500, 4000, 5000, 5500, 7500, 10000, 15000
	};
	#if TTGO	
		const float scalarsBand[13] = 
		{
			0.05f, 0.15f, 0.2f, 0.225f, 0.25f, 0.3f, 0.35f, 0.4f, 0.425f, 0.6f, 0.7f, 0.8f, 0.8f, 0.9f, 1.0f, 1.0f
		};
	#else
	// gain for each band
		const float scalarsBand[13] = 
		{
			0.2f, 0.2f, 0.25f, 0.3f, 0.5f, 0.6f, 0.7f, 0.75f, 0.6f, 0.6f, 0.7f, 0.7f, 0.7f
		};
	#endif
#endif

#if NUM_BANDS == 5
	static int cutOffsBand [5] =
	{
		100, 250, 450, 565, 715
	};
	static float scalarsBand[5] = 
	{
		0.1f, 0.2f, 0.3f, 0.3f, 0.4f
	};
#endif

#if NUM_BANDS == 8
	const int cutOffsBand[8] =
	{
		565, 900, 1400, 2250, 3150, 5000, 10000, 15000
	};
	#if TTGO	
		const float scalarsBand[7] = 
		{
			0.15f, 0.3f, 0.45f, 0.6f, 0.75f, 0.9f, 1.0f
		};
	#else
		const float scalarsBand[8] = 
		{
			0.15f, 0.3f, 0.4f, 0.5f, 0.6f, 0.75f, 0.9f, 1.0f
		};
	#endif
#endif

// BandCutoffTable
//
// Depending on how many bands we have, returns the cutoffs of where those bands are in the spectrum

const int * SampleBuffer::BandCutoffTable(int bandCount)				
{
	return cutOffsBand;
}

const float * SampleBuffer::GetBandScalars(int bandCount)
{
	return scalarsBand;		
}

#if ENABLE_AUDIO
	PeakData g_Peaks;
#endif

// AudioSamplerTaskEntry
// A background task that samples audio, computes the VU, stores it for effect use, etc.

void IRAM_ATTR AudioSamplerTaskEntry(void *)
{
	SoundAnalyzer Analyzer(INPUT_PIN);

	debugI(">>> Sampler Task Started");

	for (;;)
	{
		static uint64_t lastFrame = millis();
		g_AudioFPS = FPS(lastFrame, millis());
		lastFrame = millis();

		EVERY_N_SECONDS(2)
		{
			//debugI("Mem: %u LED FPS: %d, Audio FPS: %d, gPeakVU: %9.2f, gMinVU: %9.2f, gVURatio: %5.4f, Peaks: %8.2f, %8.2f, %8.2f, %8.2f, %8.2f", ESP.getFreeHeap(), g_FPS, g_AudioFPS, gPeakVU, gMinVU, gVURatio, g_Peaks[0], g_Peaks[1], g_Peaks[2], g_Peaks[3], g_Peaks[4]);
		}

		g_Peaks = Analyzer.RunSamplerPass();
      UpdatePeakData();        
      DecayPeaks();

		gVURatio = (gPeakVU == gMinVU) ? 0.0 : (gVU-gMinVU) / std::max(gPeakVU - gMinVU, (float) MIN_VU) * 2.0f;
		
	/*
		if(gPeakVU == gMinVU) 
		{
			gVURatio = 0;
		}
		else
		{
			gVURatio = (gVU-gMinVU) / std::max(gPeakVU - gMinVU, (float) MIN_VU) * 2.0f;
		}
	*/

		// Delay enough time to yield 25ms total used this frame, which will net 40FPS exactly (as long as the CPU keeps up)

		unsigned long elapsed = millis() - lastFrame;

		const auto targetDelay = PERIOD_FROM_FREQ(40) * MILLIS_PER_SECOND / MICROS_PER_SECOND;
		delay(elapsed >= targetDelay ? 1 : targetDelay - elapsed);

		if (g_bUpdateStarted)
			delay(1000);
	}
}
#endif

