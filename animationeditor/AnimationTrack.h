/*

Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This library contains code that was generated using ChatGPT and Copilot.

*/

#pragma once
#ifndef ANIMATION_TRACK__H
#define ANIMATION_TRACK__H

#include "AnimationEditorGlobal.h"

#include <QObject>
#include <QMap>

class AnimationEditor;
class AnimationTimelineEditor;
class AnimationCurveEditor;
class AnimationTrack;

enum class AnimationInterpolation
{
	Step,
	Linear,
	Bezier,
	TensionContinuityBias,
	EaseInOut,
};

struct AnimationKeyframe
{
	// Value on Y-axis
	double Value;

	// Interpolation data for different methods
	union InterpolationData
	{
		// Bezier handles, relative to the keyframe
		struct
		{
			double InTangentX;
			double InTangentY;
			double OutTangentX;
			double OutTangentY;
		} Bezier;

		// TCB interpolation parameters
		struct
		{
			double Tension;
			double Continuity;
			double Bias;
		} TCB;

		// Ease In/Out interpolation parameters
		struct
		{
			double EaseIn;
			double EaseOut;
		} EaseInOut;
	} Interpolation;

}; /* class AnimationKeyframe */

class ANIMATIONEDITOR_EXPORT AnimationTrack : QObject
{
	Q_OBJECT

private:
	friend AnimationEditor;
	friend AnimationTimelineEditor;
	friend AnimationCurveEditor;

	QMap<double, AnimationKeyframe> Keyframes;
	AnimationInterpolation InterpolationMethod;

	static void convertBezierToTCB(QMap<double, AnimationKeyframe> &keyframes);
	static void convertTCBToBezier(QMap<double, AnimationKeyframe> &keyframes);
	static void convertBezierToEaseInOut(QMap<double, AnimationKeyframe> &keyframes);
	static void convertEaseInOutToBezier(QMap<double, AnimationKeyframe> &keyframes);
	static void convertTCBToEaseInOut(QMap<double, AnimationKeyframe> &keyframes);
	static void convertEaseInOutToTCB(QMap<double, AnimationKeyframe> &keyframes);

	static double interpolateBezier(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t);
	static double interpolateTCB(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t);
	static double interpolateEaseInOut(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t);

}; /* class AnimationTrack */

#endif /* ANIMATION_TRACK__H */

/* end of file */
