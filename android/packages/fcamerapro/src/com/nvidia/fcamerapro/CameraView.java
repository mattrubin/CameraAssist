package com.nvidia.fcamerapro;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
//import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

// Added for GLES 2.0
import android.opengl.GLES20;
import java.io.*;
import java.io.BufferedReader;
import java.io.IOException;
import android.util.Log;

/**
 * Represents the viewfinder screen. Uses OpenGL ES.
 */
public final class CameraView extends GLSurfaceView {

	final private CameraViewRenderer mRenderer;

	public CameraView(Context context, AttributeSet attrs) {
		super(context, attrs);
		setEGLContextClientVersion(2);
		
		setEGLConfigChooser(8, 8, 8, 8, 0, 0);
		
		mRenderer = new CameraViewRenderer(context);
		setRenderer(mRenderer);
	}
	
	public void setShaderProgramIndex(int index) {
		mRenderer.setShaderProgramIndex(index);
	}

	public CameraViewRenderer getRenderer() {
		return mRenderer;
	}

	protected void onAttachedToWindow() {
		FCamInterface instance = FCamInterface.GetInstance();
		// If picture grab was completed while in viewer window, notify renderer.
		if (!instance.isCapturing()) mRenderer.onCaptureComplete();

		instance.setPreviewActive(true);
		instance.addEventListener(mRenderer);
		super.onAttachedToWindow();
	}

	protected void onDetachedFromWindow() {
		FCamInterface instance = FCamInterface.GetInstance();
		instance.setPreviewActive(false);
		instance.removeEventListener(mRenderer);
		super.onDetachedFromWindow();
	}
}

final class CameraViewRenderer implements GLSurfaceView.Renderer, FCamInterfaceEventListener {
	final private FloatBuffer mVertexBuffer, mTextureBuffer;
	final private Context mContext;
	private long mBusyTimerStart = -1;
	private int mBusyTextureId = -1;
	private int mSurfaceWidth, mSurfaceHeight;
	
	// Added for GLES 2.0
	private int mShaderProgramIndex;
	private int[] mShaderProgram;
	private int mPositionHandle;
	private int mTextureHandle;
	private int mTexCoordHandle;
	
	final static private float sVertexCoords[] = {-1.0f, 1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,  1.0f, -1.0f };
	final static private float sTextureCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	CameraViewRenderer(Context context) {
		mContext = context;

		
		ByteBuffer byteBuf = ByteBuffer.allocateDirect(sVertexCoords.length * 4);
		byteBuf.order(ByteOrder.nativeOrder());
		mVertexBuffer = byteBuf.asFloatBuffer();
		mVertexBuffer.put(sVertexCoords);
		mVertexBuffer.position(0);

		byteBuf = ByteBuffer.allocateDirect(sTextureCoords.length * 4);
		byteBuf.order(ByteOrder.nativeOrder());
		mTextureBuffer = byteBuf.asFloatBuffer();
		mTextureBuffer.put(sTextureCoords);
		mTextureBuffer.position(0);
		
		mPositionHandle = mTextureHandle = -1;
		
		mShaderProgramIndex = 0;
		/*  [CS478] Assignment #2
		 *  You'll need to modify the size of the shader program array to fit your new zebra viewfinder mode.
		 */
		mShaderProgram = new int[mContext.getResources().getStringArray(R.array.viewfinder_mode_array).length];
	}

	
	private String loadFileToString(String assetName) {
		InputStream is = null;
		try {
			StringBuffer fileData = new StringBuffer(1000);			
			is = mContext.getAssets().open(assetName);
			BufferedReader reader = new BufferedReader(new InputStreamReader(is));
			
			int bytesRead = 0;
			char[] buffer = new char[1024];
			while ((bytesRead = reader.read(buffer)) != -1) {
				fileData.append(buffer, 0, bytesRead);
			}
			reader.close();
			return fileData.toString();
		} catch (IOException e) {
			Log.e("FCamera", "Failed to find asset named " + assetName);
			return null;
		}
	}
	
	public void setShaderProgramIndex(int index) {
		mShaderProgramIndex = index;
	}
	
	public int loadShaderProgram(String shaderName) {
		int vertexShader, fragmentShader;
		
		String vertexShaderCode = loadFileToString(shaderName + ".vert");
		
		vertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
		GLES20.glShaderSource(vertexShader, vertexShaderCode);
		GLES20.glCompileShader(vertexShader);
		
		String fragmentShaderCode = loadFileToString(shaderName + ".frag");		
		fragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
		GLES20.glShaderSource(fragmentShader, fragmentShaderCode);
		GLES20.glCompileShader(fragmentShader);		
		
		int shaderProgram = GLES20.glCreateProgram();
		GLES20.glAttachShader(shaderProgram, vertexShader);
		GLES20.glAttachShader(shaderProgram, fragmentShader);
		GLES20.glLinkProgram(shaderProgram);
		
		mPositionHandle = GLES20.glGetAttribLocation(shaderProgram, "vPosition");
		mTexCoordHandle = GLES20.glGetAttribLocation(shaderProgram, "vTexCoordIn");
		mTextureHandle = GLES20.glGetAttribLocation(shaderProgram, "texture");

		return shaderProgram;
		
	}
	
	@Override
	public void onDrawFrame(GL10 gl) {
		FCamInterface iface = FCamInterface.GetInstance();

		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
		
		// Add program to OpenGL environment
		GLES20.glUseProgram(mShaderProgram[mShaderProgramIndex]);
		
		if (iface.isPreviewActive()) {		
			// lock frame texture 
			int texId = iface.lockViewerTexture();
			int texTarget = iface.getViewerTextureTarget();
			int[] activeTexture = new int[1];
			GLES20.glGetIntegerv(GLES20.GL_ACTIVE_TEXTURE, activeTexture, 0);
			
			GLES20.glEnable(texTarget);			
			GLES20.glBindTexture(texTarget, texId);
			
			
			// draw textured quad
			// Prepare the viewfinder geometry
	        GLES20.glVertexAttribPointer(mPositionHandle, 2, GLES20.GL_FLOAT, false, 0, mVertexBuffer);
	        GLES20.glEnableVertexAttribArray(mPositionHandle);
	        GLES20.glVertexAttribPointer(mTexCoordHandle, 2, GLES20.GL_FLOAT, false, 0, mTextureBuffer);
	        GLES20.glEnableVertexAttribArray(mTexCoordHandle);
	        GLES20.glUniform1i(mTextureHandle, texId);
	        
	        // Draw the viewfinder
	        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
			
			// unbind texture
	        GLES20.glBindTexture(texTarget, 0);
	        GLES20.glDisable(texTarget);
	        
			// flush render calls
	        GLES20.glFlush();
	        GLES20.glFinish();        
			
			// unlock frame texture
			iface.unlockViewerTexture();			
		}
		
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		mSurfaceWidth = width;
		mSurfaceHeight = height;
		GLES20.glViewport(0, 0, width, height);
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		
		
		mShaderProgram[0] = loadShaderProgram("default");
		/*  [CS478] Assignment #2
		 *  You should pre-load your zebra viewfinder shader program.
		 *  Loading it on demand could in theory be slow and requires
		 *  you to get control of the OpenGL thread again, which can be 
		 *  tricky.
		 */
		mShaderProgram[1] = loadShaderProgram("sharpnessguage");
		
		GLES20.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	}

	// A helper function to load file into texture.
	private int loadTextureFromResource(GL10 gl, int resourceId) {
		// Load bitmap.
		BitmapFactory.Options opts = new BitmapFactory.Options();
		opts.inScaled = false;
		Bitmap bitmap = BitmapFactory.decodeResource(mContext.getResources(), resourceId, opts);
		
		// Create texture.
		int[] tid = new int[1];
		GLES20.glGenTextures(1, tid, 0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tid[0]);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, bitmap, 0);

		// Clean up.
		bitmap.recycle();
		return tid[0];
	}
	
	// Implementation of FCamInterfaceEventListener.
	public void onCaptureStart() {
		mBusyTimerStart = System.currentTimeMillis();
	}
	public void onCaptureComplete() {
		mBusyTimerStart = -1;
	}
	public void onFileSystemChange() {}
	public void onShotParamChange(int shotParamId) {}	
}
