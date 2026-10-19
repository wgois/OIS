/*
The zlib/libpng License

Copyright (c) 2018 Arthur Brainville
Copyright (c) 2015 Andrew Fenn
Copyright (c) 2005-2010 Phillip Castaneda (pjcast -- www.wreckedgames.com)

This software is provided 'as-is', without any express or implied warranty. In no
event will the authors be held liable for any damages arising from the use of this
software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to the
following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that
        you wrote the original software. If you use this software in a product,
        an acknowledgment in the product documentation would be appreciated
        but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
 */
#ifndef OIS_CocoaMouseCoordinates_H
#define OIS_CocoaMouseCoordinates_H

#include <algorithm>
#include <cmath>
#include <limits>
#include "OISMouse.h"

namespace OIS
{
namespace CocoaPrivate
{
struct MousePosition
{
    double x, y;
};

struct ButtonSnapshot
{
    MouseButtonID button;
    MouseState state;
    MousePosition position;
    ButtonSnapshot(MouseButtonID b, const MouseState& s, MousePosition p) :
        button(b), state(s), position(p) {}
};

// A captured native delta is one logical view unit. Canvas units are application
// supplied; neither they nor native deltas imply a physical display backing scale.
class MouseCoordinates
{
public:
    MouseCoordinates() : x(0), y(0), width(0), height(0), canvasWidth(0),
        canvasHeight(0), valid(false), seeded(false) {}

    bool geometry(double w, double h, int cw, int ch)
    {
        valid = std::isfinite(w) && std::isfinite(h) && w > 0 && h > 0 && cw > 0 && ch > 0;
        if(!valid)
            return false;
        width = w; height = h; canvasWidth = cw; canvasHeight = ch;
        if(!seeded)
        {
            x = width / 2; y = height / 2; // No usable associated seed: defined center.
            seeded = true;
        }
        clamp();
        return true;
    }

    bool seed(double localX, double localY, double originX, double originY, bool flipped)
    {
        if(!valid || !std::isfinite(localX) || !std::isfinite(localY) ||
            !std::isfinite(originX) || !std::isfinite(originY))
            return false;
        x = localX - originX;
        y = flipped ? localY - originY : height - (localY - originY);
        seeded = true;
        clamp();
        return true;
    }

    void move(double dx, double dy)
    {
        if(!valid || !std::isfinite(dx) || !std::isfinite(dy))
            return;
        x += dx; y += dy;
        clamp();
    }

    bool hasGeometry() const { return valid; }
    MousePosition position() const { return MousePosition{x, y}; }

    bool project(MousePosition point, int& absX, int& absY) const
    {
        if(!valid)
            return false;
        // Historical queued points can lie outside resized bounds. Preserve their
        // logical position rather than clamping to the current virtual cursor.
        // Saturate only at the integer ABI limit if the projection is unrepresentable.
        const double limit = std::numeric_limits<int>::max();
        absX = static_cast<int>(std::min(limit, (point.x / width) * canvasWidth));
        absY = static_cast<int>(std::min(limit, (point.y / height) * canvasHeight));
        return true;
    }

    bool publish(int& absX, int& absY) const
    {
        int nextX = absX, nextY = absY;
        if(!project(position(), nextX, nextY))
            return false;
        const bool changed = absX != nextX || absY != nextY;
        absX = nextX; absY = nextY;
        return changed;
    }

private:
    void clamp()
    {
        x = std::max(0.0, std::min(x, width));
        y = std::max(0.0, std::min(y, height));
    }
    double x, y, width, height;
    int canvasWidth, canvasHeight;
    bool valid, seeded;
};
}
}
#endif
