/* Copyright (c) 1995-2010, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* Copyright (c) 2011, NVIDIA CORPORATION. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*  * Neither the name of NVIDIA CORPORATION nor the names of its
*    contributors may be used to endorse or promote products derived 
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <math.h>

#include "FCam/Tegra/AutoFocus.h"
#include "FCam/Tegra/Lens.h"
#include "FCam/Frame.h"

#include "../Debug.h"

namespace FCam { namespace Tegra {

    AutoFocus::AutoFocus(FCam::Tegra::Lens *l, Rect r) : FCam::AutoFocus(l,r) 
        {}

    void AutoFocus::startSweep() {
        if (!lens) return;
        state = HOMING;
        // focus at infinity
        lens->setFocus(lens->farFocus());
        stats.clear();
    }


    void AutoFocus::update(const Frame &f, FCam::Shot *shot) {
        if (state == FOCUSED || state == IDLE) return;

        if (state == SETTING && !lens->focusChanging())
        {
        	// Have we set the best position?
        	// or are we processing a sweeping frame?
        	float position = f["lens.focus"];
        	if (fabs(position - best.position) < 0.01 )	{
        		shot->clearActions((void *) this);
        		state = FOCUSED;
        	}
            return;
		}

        // we're sweeping or homing
        if (!f.sharpness().valid()) return;

        // convert a rect on the screen to a subset of the sharpness map
        int minSx = 0;
        int minSy = 0;
        int maxSx = f.sharpness().size().width-1;
        int maxSy = f.sharpness().size().height-1;
        float w = f.shot().image.width();
        float h = f.shot().image.height();
        if (rect.width > 0 && rect.height > 0) {
            minSx = int(rect.x * f.sharpness().size().width / w + 0.5);
            minSy = int(rect.y * f.sharpness().size().height / h + 0.5);
            maxSx = int((rect.x + rect.width) * f.sharpness().size().width / w + 0.5);
            maxSy = int((rect.y + rect.height) * f.sharpness().size().height / h + 0.5);
            if (maxSx >= f.sharpness().size().width) maxSx = f.sharpness().size().width-1;
            if (maxSy >= f.sharpness().size().height) maxSy = f.sharpness().size().height-1;
            if (minSx >= f.sharpness().size().width) minSx = f.sharpness().size().width-1;
            if (minSy >= f.sharpness().size().height) minSy = f.sharpness().size().height-1;
            if (minSx < 0) minSx = 0;
            if (minSy < 0) minSy = 0;
            if (maxSx < 0) maxSx = 0;
            if (maxSy < 0) maxSy = 0;
        }

        Stats s;
        s.position = f["lens.focus"];
        s.sharpness = 0;
        for (int sy = minSy; sy <= maxSy; sy++) {
            for (int sx = minSx; sx <= maxSx; sx++) {
                s.sharpness += f.sharpness()(sx, sy);
            }
        }
        stats.push_back(s);
        // dprintf(4, "Focus shotid: %d, position %f, sharpness %d\n", f.shot().id, s.position, s.sharpness);

        if (state == HOMING && !lens->focusChanging()) {
            // wait until we get a frame back with focus at infinity
            if (fabs(s.position - lens->farFocus()) < 0.01) {

                // do a sweep if we the minFocusSpeed allows it - otherwise, step
                if (lens->nearFocus() - lens->farFocus() > lens->minFocusSpeed()) {
                    lens->setFocus(lens->nearFocus(), (lens->nearFocus() - lens->farFocus()));
                    state = SWEEPING;
                } else {
                    state = STEPPING;

                    FCam::Lens::FocusSteppingAction stepFocus(lens);
                    stepFocus.owner = (void *) this;
                    stepFocus.time = 0;
                    stepFocus.speed = lens->maxFocusSpeed();
                    stepFocus.step = (lens->nearFocus() - lens->farFocus())/15;
                    stepFocus.repeat = 15;
                    shot->addAction(stepFocus);
                }

                return;
            }
        }

        if ((state == SWEEPING || state == STEPPING) && stats.size() > 4) {
            bool gettingWorse = true;

            // check if the sharpness is just getting continuously
            // worse (by at least 1% per tick). If so, don't wait for
            // the sweep to terminate
            for (size_t i = stats.size()-1; i > stats.size()-4; i--) {
                if (stats[i].position < stats[i-1].position ||
                        (stats[i].sharpness*101)/100 > stats[i-1].sharpness) {
                    gettingWorse = false;
                    break;
                }
            }

            // Check if it's time to set the focus. Either:
            // We have finished the entire sweep, or
            // we are stepping and the next position is out of the focal range,
            // or we are gettingWorse
            if (!lens->focusChanging() && (state == SWEEPING ||
                    (state == STEPPING && fabs(lens->getFocus() - lens->nearFocus()) < 0.01) ||
                    gettingWorse)) {

                // dprintf(4, "focus s.position = %f, lens->nearFocus = %f, gettingWorse = %d\n", 
                // s.position, lens->nearFocus(), gettingWorse);
                best = stats[0];
                for (size_t i = 1; i < stats.size(); i++) {
                    if (stats[i].sharpness > best.sharpness) best = stats[i];
                }
                
                dprintf(4, "AutoFocus best.position %f - best.sharpness %d\n", best.position, best.sharpness);
                if (state == SWEEPING) {
                    lens->setFocus(best.position);
                } else if (state == STEPPING) {
                    FCam::Lens::FocusAction setFocus(lens);
                    setFocus.owner = (void *) this;
                    setFocus.time = 0;
                    setFocus.speed = lens->maxFocusSpeed();
                    setFocus.focus = (best.position);
                    shot->clearActions((void *) this);
                    shot->addAction(setFocus);
                }
                state = SETTING;
                stats.clear();
                return;
            }
        }
    }

}}
