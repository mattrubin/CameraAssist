#include <math.h>

#include "FCam/AutoFocus.h"
#include "FCam/Lens.h"
#include "FCam/Frame.h"

#include "Debug.h"

namespace FCam {

    AutoFocus::AutoFocus(Lens *l, Rect r) : lens(l), state(IDLE), rect(r) {}
    
    void AutoFocus::startSweep() {
        if (!lens) return;
        state = HOMING;
        // focus at infinity
        lens->setFocus(lens->farFocus());
        stats.clear();
    }

    
    void AutoFocus::update(const Frame &f, FCam::Shot *shots) {
        if (state == FOCUSED || state == IDLE) return;
        
        if (state == SETTING) {
            if (!lens->focusChanging()) {
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
                s.sharpness += f.sharpness()(sx, sy) >> 10;
            }
        }
        stats.push_back(s);
        // dprintf(4, "Focus shotid: %d, position %f, sharpness %d\n", f.shot().id, s.position, s.sharpness);
        
        if (state == HOMING && !lens->focusChanging()) {
            // wait until we get a frame back with focus at infinity
            if (fabs(s.position - lens->farFocus()) < 0.01) {
                lens->setFocus(lens->nearFocus(), (lens->nearFocus() - lens->farFocus()));
                state = SWEEPING;
                return;
            }
        }
        
        if (state == SWEEPING && stats.size() > 4) {
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
            
            // Check if it's time to set the focus
            if (!lens->focusChanging() || gettingWorse) {
                Stats best = stats[0];
                for (size_t i = 1; i < stats.size(); i++) {
                    if (stats[i].sharpness > best.sharpness) best = stats[i];
                }
                
                lens->setFocus(best.position);
                state = SETTING;
                stats.clear();
                return;
            }
        }
        
        if (state == SETTING && !lens->focusChanging()) {
            state = FOCUSED;
            return;
        }
        
    }

}
