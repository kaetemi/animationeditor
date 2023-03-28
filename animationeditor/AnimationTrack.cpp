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

#include "AnimationTrack.h"

#include <QRandomGenerator>
#include <random>

// Initialize the atomic ID generator
std::atomic<ptrdiff_t> AnimationKeyframe::s_NextId(0);

AnimationTrack::AnimationTrack(QObject *parent)
    : QObject(parent)
    , m_Keyframes()
    , m_InterpolationMethod(AnimationInterpolation::Linear)
	, m_Color(Qt::white)
{
}

const QMap<double, AnimationKeyframe> &AnimationTrack::keyframes() const
{
	return m_Keyframes;
}

AnimationInterpolation AnimationTrack::interpolationMethod() const
{
	return m_InterpolationMethod;
}

static bool keyframesEqual(const AnimationKeyframe &k1, const AnimationKeyframe &k2, AnimationInterpolation interpolationMethod)
{
	bool baseEquals = k1.Value == k2.Value && k1.Id == k2.Id;
	if (interpolationMethod == AnimationInterpolation::Bezier)
	{
		return baseEquals
		    && k1.Interpolation.Bezier.InTangentX == k2.Interpolation.Bezier.InTangentX
		    && k1.Interpolation.Bezier.InTangentY == k2.Interpolation.Bezier.InTangentY
		    && k1.Interpolation.Bezier.OutTangentX == k2.Interpolation.Bezier.OutTangentX
		    && k1.Interpolation.Bezier.OutTangentY == k2.Interpolation.Bezier.OutTangentY;
	}
	else if (interpolationMethod == AnimationInterpolation::TCB)
	{
		return baseEquals
		    && k1.Interpolation.TCB.Tension == k2.Interpolation.TCB.Tension
		    && k1.Interpolation.TCB.Continuity == k2.Interpolation.TCB.Continuity
		    && k1.Interpolation.TCB.Bias == k2.Interpolation.TCB.Bias;
	}
	else if (interpolationMethod == AnimationInterpolation::EaseInOut)
	{
		return baseEquals
		    && k1.Interpolation.EaseInOut.EaseIn == k2.Interpolation.EaseInOut.EaseIn
		    && k1.Interpolation.EaseInOut.EaseOut == k2.Interpolation.EaseInOut.EaseOut;
	}
	else
	{
		return baseEquals;
	}
}

static bool mapsEqual(const QMap<double, AnimationKeyframe> &map1, const QMap<double, AnimationKeyframe> &map2, AnimationInterpolation interpolationMethod)
{
	if (map1.size() != map2.size())
	{
		return false;
	}

	QMap<double, AnimationKeyframe>::const_iterator i1 = map1.constBegin();
	QMap<double, AnimationKeyframe>::const_iterator i2 = map2.constBegin();
	while (i1 != map1.constEnd() && i2 != map2.constEnd())
	{
		if (i1.key() != i2.key() || !keyframesEqual(i1.value(), i2.value(), interpolationMethod))
		{
			return false;
		}
		++i1;
		++i2;
	}

	return true;
}

void AnimationTrack::setKeyframes(const QMap<double, AnimationKeyframe> &keyframes)
{
	// This uses existing keyframes to preserve their IDs
	if (!mapsEqual(m_Keyframes, keyframes, m_InterpolationMethod))
	{
		m_Keyframes = keyframes;
		emit keyframesChanged();
	}
}

void AnimationTrack::setInterpolationMethod(AnimationInterpolation interpolationMethod)
{
	m_InterpolationMethod = interpolationMethod;
	emit interpolationMethodChanged();
}

void AnimationTrack::upsertKeyframe(double time, const AnimationKeyframe &keyframe)
{
	QMap<double, AnimationKeyframe>::iterator it = m_Keyframes.insert(time, keyframe);
	// Assign a unique ID to the keyframe
	it->Id = AnimationKeyframe::s_NextId++;
	emit keyframesChanged();
}

void AnimationTrack::removeKeyframe(double time)
{
	if (m_Keyframes.remove(time) > 0)
	{
		emit keyframesChanged();
	}
}

void AnimationTrack::moveKeyframe(double fromTime, double toTime)
{
	if (m_Keyframes.contains(fromTime))
	{
		AnimationKeyframe keyframe = m_Keyframes.value(fromTime);
		m_Keyframes.remove(fromTime);
		m_Keyframes.insert(toTime, keyframe);
		emit keyframesChanged();
	}
}

void AnimationTrack::setColor(const QColor &color)
{
	if (m_Color != color)
	{
		m_Color = color;
		emit colorChanged();
	}
}

const QColor &AnimationTrack::color() const
{
	return m_Color;
}

void AnimationTrack::setRandomColor()
{
	static QRandomGenerator rng(static_cast<quint64>(static_cast<quint64>(std::random_device()())));

	// Generate a random hue between 0 and 359
	int hue = rng.generate() % 360;

	// Generate saturation between 80% and 90%
	int saturation = 80 + rng.generate() % 10;

	// Generate lightness between 60% and 70%
	int lightness = 60 + rng.generate() % 10;

	setColor(QColor::fromHsl(hue, saturation * 255 / 100, lightness * 255 / 100));
}

// Bezier to TCB conversion
void AnimationTrack::convertBezierToTCB(QMap<double, AnimationKeyframe> &keyframes)
{
	for (QMap<double, AnimationKeyframe>::iterator it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Compute incoming and outgoing tangents
		double inTangentX = keyframe.Interpolation.Bezier.InTangentX;
		double inTangentY = keyframe.Interpolation.Bezier.InTangentY;
		double outTangentX = keyframe.Interpolation.Bezier.OutTangentX;
		double outTangentY = keyframe.Interpolation.Bezier.OutTangentY;

		// Approximate TCB parameters based on the tangents
		// Modify these heuristics as needed to better match the desired conversion
		double tension = 1.0 - (std::abs(inTangentY) + std::abs(outTangentY)) / (std::abs(inTangentX) + std::abs(outTangentX));
		double continuity = 0.0; // Set to 0.0 as a starting point
		double bias = (inTangentY - outTangentY) / (std::abs(inTangentY) + std::abs(outTangentY));

		// Update the keyframe with TCB parameters
		keyframe.Interpolation.TCB.Tension = tension;
		keyframe.Interpolation.TCB.Continuity = continuity;
		keyframe.Interpolation.TCB.Bias = bias;
	}
}

// TCB to Bezier conversion
void AnimationTrack::convertTCBToBezier(QMap<double, AnimationKeyframe> &keyframes)
{
	for (QMap<double, AnimationKeyframe>::iterator it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Compute incoming and outgoing tangents
		// Modify these heuristics as needed to better match the desired conversion
		double inTangentX = 1.0 / (1.0 + keyframe.Interpolation.TCB.Tension);
		double inTangentY = keyframe.Interpolation.TCB.Bias * inTangentX;
		double outTangentX = 1.0 / (1.0 + keyframe.Interpolation.TCB.Tension);
		double outTangentY = -keyframe.Interpolation.TCB.Bias * outTangentX;

		// Update the keyframe with Bezier handles
		keyframe.Interpolation.Bezier.InTangentX = inTangentX;
		keyframe.Interpolation.Bezier.InTangentY = inTangentY;
		keyframe.Interpolation.Bezier.OutTangentX = outTangentX;
		keyframe.Interpolation.Bezier.OutTangentY = outTangentY;
	}
}

// Bezier to Ease In/Out conversion
void AnimationTrack::convertBezierToEaseInOut(QMap<double, AnimationKeyframe> &keyframes)
{
	for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Compute incoming and outgoing tangents
		double inTangentX = keyframe.Interpolation.Bezier.InTangentX;
		double inTangentY = keyframe.Interpolation.Bezier.InTangentY;
		double outTangentX = keyframe.Interpolation.Bezier.OutTangentX;
		double outTangentY = keyframe.Interpolation.Bezier.OutTangentY;

		// Approximate Ease In and Ease Out values based on the tangents
		double easeIn = std::abs(inTangentY / inTangentX);
		double easeOut = std::abs(outTangentY / outTangentX);

		// Update the keyframe with Ease In/Out values
		keyframe.Interpolation.EaseInOut.EaseIn = easeIn;
		keyframe.Interpolation.EaseInOut.EaseOut = easeOut;
	}
}

// Ease In/Out to Bezier conversion
void AnimationTrack::convertEaseInOutToBezier(QMap<double, AnimationKeyframe> &keyframes)
{
	for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Compute incoming and outgoing tangents
		double inTangentX = 1.0;
		double inTangentY = keyframe.Interpolation.EaseInOut.EaseIn;
		double outTangentX = 1.0;
		double outTangentY = keyframe.Interpolation.EaseInOut.EaseOut;

		// Update the keyframe with Bezier handles
		keyframe.Interpolation.Bezier.InTangentX = inTangentX;
		keyframe.Interpolation.Bezier.InTangentY = inTangentY;
		keyframe.Interpolation.Bezier.OutTangentX = outTangentX;
		keyframe.Interpolation.Bezier.OutTangentY = outTangentY;
	}
}

void AnimationTrack::convertTCBToEaseInOut(QMap<double, AnimationKeyframe> &keyframes)
{
	for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Approximate Ease In and Ease Out values based on TCB parameters
		double easeIn = 1.0 - keyframe.Interpolation.TCB.Tension;
		double easeOut = 1.0 - keyframe.Interpolation.TCB.Tension;

		// Update the keyframe with Ease In/Out values
		keyframe.Interpolation.EaseInOut.EaseIn = easeIn;
		keyframe.Interpolation.EaseInOut.EaseOut = easeOut;
	}
}

void AnimationTrack::convertEaseInOutToTCB(QMap<double, AnimationKeyframe> &keyframes)
{
	for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
	{
		AnimationKeyframe &keyframe = it.value();

		// Approximate TCB parameters based on Ease In/Out values
		double tension = 1.0 - (keyframe.Interpolation.EaseInOut.EaseIn + keyframe.Interpolation.EaseInOut.EaseOut) / 2.0;
		double continuity = 0.0; // Set to 0.0 as a starting point
		double bias = 0.0; // Set to 0.0 as a starting point

		// Update the keyframe with TCB parameters
		keyframe.Interpolation.TCB.Tension = tension;
		keyframe.Interpolation.TCB.Continuity = continuity;
		keyframe.Interpolation.TCB.Bias = bias;
	}
}

double AnimationTrack::valueAtTime(KeyframeMap::const_iterator key0, KeyframeMap::const_iterator key1, double time) const
{
	// If the time is before the first keyframe, return the first keyframe's value
	if (key0 != m_Keyframes.end() && time < key0.key())
		return key0.value().Value;

	// If the time is after the second keyframe, return the second keyframe's value
	if (key1 != m_Keyframes.end() && time > key1.key())
		return key0.value().Value;

	// If the time is between the two keyframes, interpolate between them
	if (key0 != m_Keyframes.end() && key1 != m_Keyframes.end())
	{
		// If the keyframes are the same, return the value
		if (key0.key() == key1.key())
			return key0.value().Value;

		// If the keyframes are not the same, interpolate between them
		switch (m_InterpolationMethod)
		{
		// case AnimationInterpolation::Linear:
		// 	return interpolateLinear(key0.key(), key0.value(), key1.key(), key1.value(), time);
		case AnimationInterpolation::Bezier:
			return interpolateBezier(key0.key(), key0.value(), key1.key(), key1.value(), time);
		case AnimationInterpolation::TCB:
			return interpolateTCB(key0.key(), key0.value(), key1.key(), key1.value(), time);
		case AnimationInterpolation::EaseInOut:
			return interpolateEaseInOut(key0.key(), key0.value(), key1.key(), key1.value(), time);
		}
	}
}

double AnimationTrack::interpolateBezier(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t)
{
	// Normalize the interpolation time to the range [0, 1]
	double normalizedT = (t - t0) / (t1 - t0);

	// Compute the Bezier handle positions (absolute)
	double handle0_x = t0 + k0.Interpolation.Bezier.OutTangentX;
	double handle0_y = k0.Value + k0.Interpolation.Bezier.OutTangentY;
	double handle1_x = t1 + k1.Interpolation.Bezier.InTangentX;
	double handle1_y = k1.Value + k1.Interpolation.Bezier.InTangentY;

	// Perform the cubic Bezier interpolation
	double oneMinusT = 1.0 - normalizedT;
	double oneMinusTSquared = oneMinusT * oneMinusT;
	double oneMinusTCubed = oneMinusTSquared * oneMinusT;
	double tSquared = normalizedT * normalizedT;
	double tCubed = tSquared * normalizedT;

	double interpolatedValue = oneMinusTCubed * k0.Value + 3.0 * oneMinusTSquared * normalizedT * handle0_y + 3.0 * oneMinusT * tSquared * handle1_y + tCubed * k1.Value;

	return interpolatedValue;
}

double AnimationTrack::interpolateTCB(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t)
{
	// Normalize the interpolation time to the range [0, 1]
	double normalizedT = (t - t0) / (t1 - t0);

	// Compute the tension, continuity, and bias parameters
	double t0_tension = (1.0 - k0.Interpolation.TCB.Tension) * 0.5;
	double t1_tension = (1.0 - k1.Interpolation.TCB.Tension) * 0.5;
	double t0_bias = (1.0 + k0.Interpolation.TCB.Bias) * t0_tension;
	double t1_bias = (1.0 - k1.Interpolation.TCB.Bias) * t1_tension;
	double t0_continuity = (1.0 - k0.Interpolation.TCB.Continuity) * 0.5;
	double t1_continuity = (1.0 + k1.Interpolation.TCB.Continuity) * 0.5;

	// Compute the incoming and outgoing tangents
	double outTangent0 = (k1.Value - k0.Value) * t0_bias * t0_continuity;
	double inTangent1 = (k1.Value - k0.Value) * t1_bias * t1_continuity;

	// Perform cubic Hermite interpolation
	double oneMinusT = 1.0 - normalizedT;
	double oneMinusTSquared = oneMinusT * oneMinusT;
	double tSquared = normalizedT * normalizedT;

	double h00 = 2.0 * tSquared * oneMinusT + tSquared - oneMinusT;
	double h10 = tSquared * oneMinusT;
	double h01 = -2.0 * tSquared * oneMinusT + tSquared;
	double h11 = tSquared * oneMinusT;

	double interpolatedValue = h00 * k0.Value + h10 * outTangent0 * (t1 - t0) + h01 * k1.Value + h11 * inTangent1 * (t1 - t0);

	return interpolatedValue;
}

double AnimationTrack::interpolateEaseInOut(double t0, const AnimationKeyframe &k0, double t1, const AnimationKeyframe &k1, double t)
{
	// Normalize the interpolation time to the range [0, 1]
	double normalizedT = (t - t0) / (t1 - t0);

	// Compute the incoming and outgoing tangents
	double outTangent0 = k0.Interpolation.EaseInOut.EaseOut * (k1.Value - k0.Value);
	double inTangent1 = k1.Interpolation.EaseInOut.EaseIn * (k1.Value - k0.Value);

	// Perform cubic Hermite interpolation
	double oneMinusT = 1.0 - normalizedT;
	double oneMinusTSquared = oneMinusT * oneMinusT;
	double tSquared = normalizedT * normalizedT;

	double h00 = 2.0 * tSquared * oneMinusT + tSquared - oneMinusT;
	double h10 = tSquared * oneMinusT;
	double h01 = -2.0 * tSquared * oneMinusT + tSquared;
	double h11 = tSquared * oneMinusT;

	double interpolatedValue = h00 * k0.Value + h10 * outTangent0 * (t1 - t0) + h01 * k1.Value + h11 * inTangent1 * (t1 - t0);

	return interpolatedValue;
}

void AnimationTrack::convertInterpolation(QMap<double, AnimationKeyframe> &keyframes, AnimationInterpolation from, AnimationInterpolation to)
{
	if (from == to)
	{
		return; // No conversion needed
	}

	// Temporary QMap to store converted keyframes
	QMap<double, AnimationKeyframe> convertedKeyframes;

	// Convert 'from' interpolation method to Bezier if necessary
	if (from != AnimationInterpolation::Bezier)
	{
		switch (from)
		{
		case AnimationInterpolation::TCB:
			convertTCBToBezier(keyframes);
			break;
		case AnimationInterpolation::EaseInOut:
			convertEaseInOutToBezier(keyframes);
			break;
			// Add cases for other interpolation methods if needed
		default:
			// Set keyframes to Bezier with linear tangents as a reasonable default
			for (auto &keyframe : keyframes)
			{
				keyframe.Interpolation.Bezier.InTangentX = 1.0;
				keyframe.Interpolation.Bezier.InTangentY = 0.0;
				keyframe.Interpolation.Bezier.OutTangentX = 1.0;
				keyframe.Interpolation.Bezier.OutTangentY = 0.0;
			}
			break;
		}
	}

	// At this point, all keyframes are in Bezier format
	// Convert Bezier keyframes to the 'to' interpolation method if necessary
	if (to != AnimationInterpolation::Bezier)
	{
		switch (to)
		{
		case AnimationInterpolation::TCB:
			convertBezierToTCB(keyframes);
			break;
		case AnimationInterpolation::EaseInOut:
			convertBezierToEaseInOut(keyframes);
			break;
		}
	}
}

/*

# ChatGPT Dump

We have developed a time scrubber for an animation system with support for different interpolation methods, including Bezier, TCB (Tension, Continuity, Bias), and Ease In/Out. The core component of the system is the `AnimationKeyframe` class, which stores keyframe data and interpolation parameters specific to each method. The class uses a union to store these parameters efficiently.

We have implemented functions to convert between the different interpolation methods, enabling seamless transitions and providing flexibility for animators. These functions are:
1. `convertBezierToTCB` and `convertTCBToBezier`
2. `convertBezierToEaseInOut` and `convertEaseInOutToBezier`
3. `convertTCBToEaseInOut` and `convertEaseInOutToTCB`

We have also implemented interpolation functions for each method, which calculate the interpolated value given two keyframes and a time value:
1. `interpolateBezier`
2. `interpolateTCB`
3. `interpolateEaseInOut`

The system is functional and provides a good foundation for building upon. However, further improvements can be planned and implemented, such as:
1. Optimizing the conversion and interpolation functions for better performance.
2. Implementing additional interpolation methods, like Quaternion Slerp for rotations or other spline-based interpolations.
3. Enhancing the user interface for the time scrubber and easing the process of selecting and modifying keyframes.
4. Adding support for keyframe channels, which would allow for simultaneous manipulation of multiple properties.

This summary should provide a clear understanding of the current state of the time scrubber and its underlying components, enabling the planning of future improvements and extensions.

- Consider using QPointF for Bezier handle representation:
Instead of using separate variables for X and Y coordinates of InTangent and OutTangent,
you can use QPointF from the Qt library to represent the coordinates as a single object.

*/

/* end of file */
