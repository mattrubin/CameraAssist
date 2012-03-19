#ifndef _MYAUTOFOCUS_H
#define _MYAUTOFOCUS_H

#include <FCam/Tegra/AutoFocus.h>
#include <FCam/Tegra/Lens.h>
#include <FCam/Base.h>
#include <android/log.h>
#include <time.h>
#include <math.h>


#define BIG_STEP_COUNT 10.0f
#define SMALL_STEP_COUNT 50.0f
#define FOCUS_TOLERANCE 0.005f

#define FAST_BIG_STEP_COUNT 10.0f
#define FAST_SMALL_STEP_COUNT 25.0f
#define FAST_FOCUS_TOLERANCE 0.025f

#define SPOT_RADIUS 50

#define TEST_LOOP false
#define SAMPLE_SIZE 50

class MyAutoFocus : public FCam::Tegra::AutoFocus {
public:
       MyAutoFocus(FCam::Tegra::Lens *l, FCam::Rect r = FCam::Rect()) : FCam::Tegra::AutoFocus(l,r) {
    	   lens = l;
    	   rect = r;

    	   focusing = false;
    	   speedBoost = false;

           nearFocus = lens->nearFocus();
           farFocus = lens->farFocus();

           spotX = -1;
           spotY = -1;

           if(TEST_LOOP) runCount = 0;
       }

       void startSweep() {
    	   if (!lens) return;

    	   // Only initialize if the AF sweep isn't already in progress
    	   if(focusing) return;

    	   // Set up the sweep
    	   if(speedBoost){
    		   step = (nearFocus-farFocus)/FAST_BIG_STEP_COUNT;
    		   fineStep = (nearFocus-farFocus)/FAST_SMALL_STEP_COUNT;
    	   } else {
    		   step = (nearFocus-farFocus)/BIG_STEP_COUNT;
    		   fineStep = (nearFocus-farFocus)/SMALL_STEP_COUNT;
    	   }

    	   focusing = true;
    	   sweepNum = 0;

    	   targetFocus = farFocus;

    	   lastBestFocus = farFocus;
    	   lastBestContrast = 0;
    	   fineLowerBound = 0;

    	   // Begin the sweep
    	   if(speedBoost){
        	   LOG("AF: START FAST");
    	   } else if(spotX>=0) {
        	   LOG("Autofocus: START %i, %i", spotX, spotY);
    	   } else {
        	   LOG("AF: START");
    	   }
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
    	   if(!equalFocus(lastFocus, targetFocus)){
        	   lens->setFocus(targetFocus);
    		   LOG("AF: Moving: %f >> %f", lastFocus, targetFocus);
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
    		   if(contrast>lastBestContrast || equalFocus(lastFocus, farFocus+step)){
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
        				   finish();
        				   return;
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
    	        	   finish();
    	        	   return;
    			   }
    		   }


    	   }
       }


       void runFaster(bool b = true){
    	   speedBoost = b;
       }

       void setSpot(float x = -1, float y = -1){
    	   spotX = x;
    	   spotY = y;
    	   LOG("setSpot %f %f", spotX, spotY);
       }


private:
       FCam::Tegra::Lens* lens;
       FCam::Rect rect;

       bool focusing; // Whether the autofocus is running
       bool speedBoost; // Whether the faster (less precise) version is active


       int sweepNum; // 0 is the first slow sweep, 1 is the second finer sweep
       float targetFocus; // The next focus to check

       float nearFocus; // The near (high) focus limit of the lens
       float farFocus; // The far (low) focus limit of the lens

       float step; // The step (in diopters) between frames on the first sweep
       float fineStep; // The step (in diopters) between frames on the second sweep

       float lastBestFocus; // The focus of the frame with the best contrast so far
       float lastBestContrast; // The contrast measurement of the frame with the best contrast so far
       float fineLowerBound; // The focus of the frame just before the best one so far; used as the far boundary of the second sweep

       double startTime; // The starting time in seconds, used for measuring the speed
       int runCount; // how many times the autofocus has been run since initialization, used for running speed tests

       float spotX;
       float spotY;

       void finish(){
		   focusing = false;

		   LOG("AF: Done: %f - %.6lf", lastBestFocus, elapsedTime());
		   LOG("AF: COMPLETE");

		   if(TEST_LOOP) runCount++;
		   if(TEST_LOOP && runCount<SAMPLE_SIZE) startSweep();
       }

       uint calculateContrast(const FCam::Image &image){
    	   if(!image.valid()) return -1;
    	   uchar *src = (uchar *)image(0, 0);
    	   uint contrast = 0;

    	   uint width = image.width();
    	   uint height = image.height();

		   int startX = 0;
		   int endX = width;
		   int startY = 0;
		   int endY = height;

    	   if(spotX>=0){
    		   startX = spotX*width-SPOT_RADIUS;
    		   if(startX<0) startX = 0;
    		   endX = spotX*width+SPOT_RADIUS;
    		   if(endX>width) endX = width;

    		   startY = spotY*height-SPOT_RADIUS;
    		   if(startY<0) startY = 0;
    		   endY = spotY*height+SPOT_RADIUS;
    		   if(endY>height) endY = height;
    	   }

    	   LOG("AF:PROCESS: Looking at Spot px (%u-%u, %u-%u)   -   (%f, %f)", startX, endX, startY, endY, spotX, spotY);
    	   for(uint row = startY; row<endY; row++){
    		   for(uint col = startX; col<endX; col++){
    			   if(col<endX-1)
    				   contrast += abs(src[row*width+col]-src[row*width+(col+1)]);
    			   if(!speedBoost && row<endY+1) // if speed boost is active, only measure the horizontal contrast
    				   contrast += abs(src[row*width+col]-src[(row+1)*width+col]);
    		   }
    	   }

    	   return contrast;
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
       double elapsedTime(){
		   timeval tim;
           gettimeofday(&tim, NULL);
           return (tim.tv_sec+(tim.tv_usec/1000000.0))-startTime;
       }


       bool equalFocus(float a, float b){
    	   return (fabs(a - b) < FOCUS_TOLERANCE);
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

};

#endif
