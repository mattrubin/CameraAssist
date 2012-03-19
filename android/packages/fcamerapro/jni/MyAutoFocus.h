#ifndef _MYAUTOFOCUS_H
#define _MYAUTOFOCUS_H

#include <FCam/Tegra/AutoFocus.h>
#include <FCam/Tegra/Lens.h>
#include <FCam/Base.h>
#include <android/log.h>
#include <time.h>

#define BIG_STEP_COUNT 10.0f
#define SMALL_STEP_COUNT 50.0f

class MyAutoFocus : public FCam::Tegra::AutoFocus {
public:
       MyAutoFocus(FCam::Tegra::Lens *l, FCam::Rect r = FCam::Rect()) : FCam::Tegra::AutoFocus(l,r) {
    	   lens = l;
    	   rect = r;
    	   /* [CS478]
    	    * Do any initialization you need.
    	    */
    	   focusing = false;

           nearFocus = lens->nearFocus();
           farFocus = lens->farFocus();

           step = (nearFocus-farFocus)/BIG_STEP_COUNT;
           fineStep = (nearFocus-farFocus)/SMALL_STEP_COUNT;
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
    	   LOG("AF: START");
    	   focusing = true;
    	   sweepNum = 0;
    	   targetFocus = farFocus;
    	   lastBestFocus = farFocus;
    	   lastBestContrast = 0;
    	   fineLowerBound = 0;

    	   startTimer();

       }

       void cancelSweep() {
    	   if(!focusing) return;
    	   // Cancel the sweep
    	   LOG("AF: Cancelled");
    	   focusing = false;
       }


       void update(const FCam::Frame &f) {
    	   /* [CS478]
    	    * This method is supposed to be called in order to inform
    	    * the autofocus engine of a new viewfinder frame.
    	    * You probably want to compute how sharp it is, and possibly
    	    * use the information to plan where to position the lens next.
    	    */

    	   // If autofocus is not in progress, return immediately
    	   if(!focusing) return;

    	   // Get the focus of the current frame
    	   FCam::Lens::Tags::Tags t = FCam::Lens::Tags::Tags(f);
    	   float lastFocus = t.focus;

    	   // If the frame was not taken at the target focus distance, (continue to) move the lens
    	   if(lastFocus != targetFocus){
        	   lens->setFocus(targetFocus);
    		   LOG("AF: Moving: %.2f >> %.2f", lastFocus, targetFocus);
    	   }

    	   // Otherwise, analyze the frame and move the focus one step closer
    	   else {

			   // Preemptively move the lens to the next position
    		   // This way, the lens can already be moving to the next position while we process this image
    		   // It won't always be right, but it saves time over all
			   lens->setFocus(nextFocus());

    		   // Calculate the contrast
    		   uint contrast = calculateContrast(f.image());

    		   // If the image can't be analyzed, return the function and try again on the next frame
    		   // If this ever gets stuck here, it's bad, but it means there's a larger problem going on with the camera
    		   if(contrast == -1){
        		   LOG("AF: Contrast: %.3f diopters: Invalid Image!", lastFocus);
    			   return;
    		   }
    		   LOG("AF: Contrast: %.3f diopters: %u", lastFocus, contrast);

    		   // Compare this frame to the last one for contrast
    		   if(contrast>lastBestContrast || lastFocus == farFocus+step){
    			   // The contrast improved
        		   LOG("AF: Improvement %.3f + %.3f", lastBestFocus, lastFocus);
        		   // If this is the first sweep, update the lower boundary of the second sweep
    			   if(sweepNum == 0) fineLowerBound = lastBestFocus;
    			   // Store the new best focus
    			   lastBestFocus = lastFocus;
    			   lastBestContrast = contrast;

    			   // Move the lens to the next position
        		   if(sweepNum == 0){
        			   if(lastFocus >= nearFocus){
        				   // If the lens has reached the end of the sweep, begin the second sweep
        				   sweepNum = 1;
        		    	   lastBestFocus = lastFocus;
        		    	   lastBestContrast = 0;

        				   LOG("AF: Defaulting to Phase 2");
        			   } else {
        				   // Set the next target focus
        				   targetFocus = targetFocus+step;
        				   if(targetFocus >= nearFocus){
        					   targetFocus = nearFocus;
        				   }
        				   lens->setFocus(targetFocus);
        				   //LOG("AF: %.3f %+.2f = %.3f", lastFocus, step, targetFocus);
        			   }
        		   } else if(sweepNum == 1){
        			   if(lastFocus <= fineLowerBound){
        				   // If the lens has reached the end of the sweep, stop
        				   focusing = false;
        				   LOG("AF: Failed");
        				   endTimer();
        				   LOG ("AF:");
        			   } else {
        				   // Set the next target focus
        				   targetFocus = targetFocus-fineStep;
        				   if(targetFocus <= farFocus){
        					   targetFocus = farFocus;
        				   }
        				   lens->setFocus(targetFocus);
        				   //LOG("AF: %.3f %+.2f = %.3f", lastFocus, -fineStep, targetFocus);
        			   }
        		   }

    		   } else {
    			   // The contrast got worse
        		   LOG("AF: Decrease %.3f - %.3f", lastBestFocus, lastFocus);

    			   if(sweepNum == 0){
    				   // begin the fine sweep back
    				   sweepNum = 1;
    		    	   lastBestFocus = lastFocus;
    		    	   lastBestContrast = 0;
    				   LOG("AF: Phase 2");

    			   } else if (sweepNum == 1) {
    				   // If this is the second sweep
    				   // set the lens to the best focus we could find
    	        	   lens->setFocus(lastBestFocus);
    	        	   // Stop autofocusing
    				   focusing = false;
    				   LOG("AF: Complete");

    				   endTimer();
    				   LOG ("AF:");

    				   return;
    			   }
    		   }


    	   }
       }

       void startTimer(){
           timeval tim;
           gettimeofday(&tim, NULL);
           startTime=tim.tv_sec+(tim.tv_usec/1000000.0);
       }
       void endTimer(){
		   timeval tim;
           gettimeofday(&tim, NULL);
           double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
           LOG("AF: Time: %.6lf seconds elapsed\n", t2-startTime);

       }


       float nextFocus(){
    	   if(sweepNum == 0){
			   float next = targetFocus+step;
			   if(next >= nearFocus)
				   return nearFocus;
			   return next;
		   } else if(sweepNum == 1){
			   float next = targetFocus-fineStep;
			   if(next <= farFocus)
				   return farFocus;
			   return next;
		   }
		   // failsafe
		   return targetFocus;
       }

       uint calculateContrast(const FCam::Image &image){
    	   if(!image.valid()) return -1;
    	   uchar *src = (uchar *)image(0, 0);
    	  // LOG("AF: %u x %u = %u  --> %u", image.width(), image.height(), image.width()*image.height(), (uint(image.width()*image.height()))*255);
    	  // LOG("AF: %x, %x, %x, %x", src[0], src[1], src[2], src[3]);
    	   uint contrast = 0;

    	   uint height = image.height();
    	   uint width = image.width();
    	   for(uint row = 0; row<height; row++){
    		   for(uint col = 0; col<width; col++){
    			   if(col<width-1)
    				   contrast += abs(src[row*width+col]-src[row*width+(col+1)]);
    			   if(row<height+1)
    				   contrast += abs(src[row*width+col]-src[(row+1)*width+col]);
    		   }
    	   }


    	   return contrast;
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

       float lastBestFocus;
       float lastBestContrast;
       float fineLowerBound;

       double startTime;
};

#endif
