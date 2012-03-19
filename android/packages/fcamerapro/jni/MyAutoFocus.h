#ifndef _MYAUTOFOCUS_H
#define _MYAUTOFOCUS_H

#include <FCam/Tegra/AutoFocus.h>
#include <FCam/Tegra/Lens.h>
#include <FCam/Base.h>
#include <android/log.h>

class MyAutoFocus : public FCam::Tegra::AutoFocus {
public:
       MyAutoFocus(FCam::Tegra::Lens *l, FCam::Rect r = FCam::Rect()) : FCam::Tegra::AutoFocus(l,r) {
    	   lens = l;
    	   rect = r;
    	   /* [CS478]
    	    * Do any initialization you need.
    	    */
    	   focusing = false;

           step = 1.0f;
           fineStep = 0.1f;
           nearFocus = 10.0f;
           farFocus = 0.0f;
       }

       void startSweep() {
    	   if (!lens) return;
    	   /* [CS478]
    	    * This method should initiate your autofocus algorithm.
    	    * Before you do that, do basic checks, e.g. is the autofocus
    	    * already engaged?
    	    */

    	   // Only initialize if the AF sweep isn't already in progress
    	   if(focusing) return;

    	   // Begin the sweep
    	   focusing = true;
    	   sweepNum = 0;
    	   targetFocus = farFocus;
    	   LOG("Autofocus: START");
       }

       void cancelSweep() {
    	   if(!focusing) return;
    	   focusing = false;
    	   LOG("Autofocus: Cancelled");
       }


       void update(const FCam::Frame &f) {
    	   /* [CS478]
    	    * This method is supposed to be called in order to inform
    	    * the autofocus engine of a new viewfinder frame.
    	    * You probably want to compute how sharp it is, and possibly
    	    * use the information to plan where to position the lens next.
    	    */

    	   // Get the focus of the current frame
    	   FCam::Lens::Tags::Tags t = FCam::Lens::Tags::Tags(f);
    	   float lastFocus = t.focus;

    	   // If the frame was not taken at the target focus distance, move the lens
    	   if(lastFocus != targetFocus){
        	   lens->setFocus(targetFocus);
    		   LOG("Autofocus: %.2f >> %.2f", lastFocus, targetFocus);
    	   }
    	   // Otherwise, analyze the frame and move the focus one step closer
    	   else {
    		   // TODO: Analyze the focus

    		   if(sweepNum == 0){
    			   if(lastFocus >= nearFocus){
    				   // If the lens has reached the end of the sweep, stop
    				   sweepNum = 1;
    				   LOG("Autofocus: Phase 2");
    			   } else {
    				   // Set the next target focus
    				   targetFocus = targetFocus+step;
    				   if(targetFocus >= nearFocus){
    					   targetFocus = nearFocus;
    				   }
    				   lens->setFocus(targetFocus);
    				   LOG("Autofocus: %.3f %+.2f = %.3f", lastFocus, step, targetFocus);
    			   }
    		   } else if(sweepNum == 1){
    			   if(lastFocus <= farFocus){
    				   // If the lens has reached the end of the sweep, stop
    				   focusing = false;
    				   LOG("Autofocus: Complete");
    			   } else {
    				   // Set the next target focus
    				   targetFocus = targetFocus-fineStep;
    				   if(targetFocus <= farFocus){
    					   targetFocus = farFocus;
    				   }
    				   lens->setFocus(targetFocus);
    				   LOG("Autofocus: %.3f %+.2f = %.3f", lastFocus, -fineStep, targetFocus);
    			   }
    		   }
    	   }
       }

private:
       FCam::Tegra::Lens* lens;
       FCam::Rect rect;
       /* [CS478]
        * Declare any state variables you might need here.
        */
       bool focusing;
       int sweepNum;
       float targetFocus;

       float nearFocus;
       float farFocus;

       float step;
       float fineStep;
};

#endif
