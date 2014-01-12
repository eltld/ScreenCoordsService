package me.wtao.service;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

import me.wtao.utils.Logcat;
import me.wtao.utils.ScreenMetrics;
import me.wtao.utils.TouchDeviceParser;
import me.wtao.view.SystemOverlayView;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Service;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.MotionEvent.PointerProperties;

public class ScreenCoordsService extends Service {	
	public static interface OnScreenTouchListener {
		public void onScreenTouch(MotionEvent event);
	}
	
	private static final int AXIS_X = 0;	// MotionEvent.AXIS_X
	private static final int AXIS_Y = 1;	// MotionEvent.AXIS_Y
	
	private static final int SOURCE_CLASS_POINTER = 0x00000002;
	private static final int SOURCE_TOUCHSCREEN = 0x00001000 | SOURCE_CLASS_POINTER;
	
	private static final TouchDeviceParser sDevice = TouchDeviceParser.getTouchDeviceParser();
	
	private static final boolean sPerformanceAccelerate = true;
	private static final Logcat sLogcat = new Logcat();
	static {
		sLogcat.setOn();	// debug mode enable
	}

	private WeakReference<NativeEventParserDaemon> mDaemonRef;
	private ArrayList<WeakReference<OnScreenTouchListener>> mOnScreenTouchListeners;
	private SystemOverlayView mOverlayView;
	
	@Override
	public void onCreate() {
		sLogcat.d("entry");
		
		mDaemonRef = NativeEventParserDaemon.getDaemonRef(this);
		if(mDaemonRef.get() == null) {
			sLogcat.e("failed to ref the daemon!");
		}
		Thread.State state = mDaemonRef.get().getState();
		if (state == Thread.State.NEW) {				
			mDaemonRef.get().start();
		}
		
		mOnScreenTouchListeners = new ArrayList<WeakReference<OnScreenTouchListener>>();
		
		mOverlayView = new SystemOverlayView(this);
		mOverlayView.attachedToWindow();
		mOverlayView.show();
		
		super.onCreate();
	}
	
	@Override
	public void onDestroy() {
		sLogcat.d("entry");

//		mDaemonRef.get().recycle(); // must before stop()
//		mDaemonRef.get().stop(); // TODO: crash, need stop with safety
		
		mOverlayView.dismiss();
		
		super.onDestroy();
	}
	
	@Override
	public IBinder onBind(Intent intent) {
		Stub stub = new Stub();
		setOnScreenTouchListener(stub);
		return stub;
	}
	
	public void setOnScreenTouchListener(OnScreenTouchListener listener) {
		if (listener instanceof OnScreenTouchListener) {
			synchronized (mOnScreenTouchListeners) {
				mOnScreenTouchListeners
						.add(new WeakReference<OnScreenTouchListener>(listener));
			}
		}
	}
	
	public void notifyAllListeners() {
		sLogcat.v("start...");
		
		MotionEvent event = packetEvent();
		
		synchronized (mOnScreenTouchListeners) {
			ArrayList<WeakReference<OnScreenTouchListener>> invalied = 
					new ArrayList<WeakReference<OnScreenTouchListener>>();
			for(WeakReference<OnScreenTouchListener> listener : mOnScreenTouchListeners) {
				if(listener.get() == null) {
					invalied.add(listener);
				} else {
					listener.get().onScreenTouch(event);
				}
			}
			
			if (!invalied.isEmpty()) {
				mOnScreenTouchListeners.removeAll(invalied);
			}
		}
		
		mDaemonRef.get().ack();
		
		sLogcat.v("ok.");
	}
	
	@SuppressLint("Recycle")
	@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
	private MotionEvent packetEvent() {
		final long current = System.currentTimeMillis();
		final long offset = current - getDownTime();
		MotionEvent event = null;
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			event = MotionEvent.obtain(current,
					getEventTime() + offset,
					getAction(),
					getPointerCount(),
					getPointerProperties(),
					getPointerCoords(),
					getMetaState(),
					getButtonState(),
					getPrecisionX(),
					getPrecisionY(),
					getDeviceId(),
					getEdgeFlags(),
					getSource(),
					getFlags());
		} else if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
			event = MotionEvent.obtain(current,
					getEventTime() + offset,
					getAction(),
					getPointerCount(),
					getPointerIds(),
					getPointerCoords(),
					getMetaState(),
					getPrecisionX(),
					getPrecisionY(),
					getDeviceId(),
					getEdgeFlags(),
					getSource(),
					getFlags());
		} else {
			sLogcat.w("version is too low: api " + Build.VERSION.SDK_INT);
		}
		return event;
	}
	
	private long getDownTime() {
		return mDaemonRef.get().getDownTime();
	}

	private long getEventTime() {
		return mDaemonRef.get().getEventTime();
	}

	private int getAction() {
		return mDaemonRef.get().getAction();
	}

	private int getPointerCount() {
		return mDaemonRef.get().getPointerCount();
	}
	
	private int[] getPointerIds() {
		return mDaemonRef.get().getPointerIds();
	}

	@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
	private PointerProperties[] getPointerProperties() {
		int[] ids = getPointerIds();
		PointerProperties[] props = new PointerProperties[ids.length];
		
		for(int i = 0; i != ids.length; ++i) {
			props[i] = new PointerProperties();
			props[i].id  = ids[i];
			props[i].toolType = MotionEvent.TOOL_TYPE_UNKNOWN;
		}
		return props;
	}

	@TargetApi(Build.VERSION_CODES.GINGERBREAD)
	private PointerCoords[] getPointerCoords() {
		PointerCoords[] coords = mDaemonRef.get().getPointerCoords();
		
		final float width = sDevice.getWidth();
		final float height = sDevice.getHeight();
		
		ScreenMetrics metrics = new ScreenMetrics(this);
		final int orientation = metrics.getOrientation();
		
		float tmp;
		
		for(PointerCoords coord : coords) {
			switch (orientation) {
			case ScreenMetrics.NO_ROTATION:
				// nothing changed
				break;
			case ScreenMetrics.ROTATION_90:
				tmp = coord.x;
				coord.x = coord.y;
				coord.y = width - tmp;
				break;
			case ScreenMetrics.ROTATION_180:
				coord.x = width - coord.x;
				coord.y = height - coord.y;
				break;
			case ScreenMetrics.ROTATION_270:
				tmp = coord.y;
				coord.y = coord.x;
				coord.x = height - tmp;
				break;
			}
		}
		
		return coords;
	}

	private int getMetaState() {
		// just ignored, not support key event so far
		return 0;
	}

	private int getButtonState() {
		// just ignored, not support key event so far
		return 0;
	}

	@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
	private float getPrecisionX() {
		return mDaemonRef.get().getPrecision(AXIS_X);
	}

	@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
	private float getPrecisionY() {
		return mDaemonRef.get().getPrecision(AXIS_Y);
	}

	private int getDeviceId() {
		return sDevice.getTouchDeviceId();
	}

	@TargetApi(Build.VERSION_CODES.GINGERBREAD)
	private int getEdgeFlags() {
		int flag = 0; // AMOTION_EVENT_EDGE_FLAG_NONE
		
		if (sPerformanceAccelerate) {
			// for performance, ignore the edge flag
			return flag;
		}
		
		Rect rect = new Rect(mOverlayView.getRight(),
				mOverlayView.getBottom(),
				mOverlayView.getLeft(),
				mOverlayView.getTop());

		PointerCoords[] coords = getPointerCoords();
		for(PointerCoords coord : coords) {
			if(coord.x < rect.left) {
				rect.left = (int) (coord.x + 0.5f);
			} else if(coord.x > rect.right){
				rect.right = (int) (coord.x + 0.5f);
			}
			
			if(coord.y < rect.top) {
				rect.top = (int) (coord.y + 0.5f);
			} else if(coord.y > rect.bottom){
				rect.bottom = (int) (coord.y + 0.5f);
			}
		}
		
		if (rect.left == mOverlayView.getLeft()) {
			flag |= MotionEvent.EDGE_LEFT;
		}
		if (rect.left == mOverlayView.getTop()) {
			flag |= MotionEvent.EDGE_TOP;
		}
		if (rect.left == mOverlayView.getRight()) {
			flag |= MotionEvent.EDGE_RIGHT;
		}
		if (rect.left == mOverlayView.getBottom()) {
			flag |= MotionEvent.EDGE_BOTTOM;
		}
		
		return flag;
	}

	private int getSource() {
		return SOURCE_TOUCHSCREEN;
	}

	private int getFlags() {
		// MotionEvent.FLAG_WINDOW_IS_OBSCURED ignored, which is for security check
		return 0;
	}
	
	private class Stub extends IScreenCoordsService.Stub implements OnScreenTouchListener {
		private MotionEvent mEvent;
		
		@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
		@SuppressLint("Recycle")
		@Override
		public MotionEvent obtainMotionEvent() throws RemoteException {
			try {
				synchronized (this) {
					ScreenCoordsService.sLogcat.v("bolcked and wait...");
					wait();
					ScreenCoordsService.sLogcat.v("fetch event and return...");
					return mEvent;
				}
			} catch (Exception e) {
				sLogcat.w(e);
				throw new RemoteException(e.toString() + e.getMessage());
			}
		}

		@Override
		public void onScreenTouch(MotionEvent event) {
			synchronized (this) {
				mEvent = event;
				notify();
			}
		}
		
	}
	
	private static class NativeEventParserDaemon extends Thread {
		private static final long TIME_OUT_MILLIS = 10000; // 5-10s ANR
		
		private static NativeEventParserDaemon sDaemon = null;
		private WeakReference<ScreenCoordsService> mServiceRef;
		private volatile Boolean mAvaiable = false;
		
		public static WeakReference<NativeEventParserDaemon> getDaemonRef(ScreenCoordsService service) {
			if(sDaemon == null) {
				synchronized(NativeEventParserDaemon.class) {
					if(sDaemon == null) {
						sDaemon = new NativeEventParserDaemon();
					}
				}
			}
			sDaemon.forceResetService(service);
			return new WeakReference<NativeEventParserDaemon>(sDaemon);
		}
		
		@Override
		public synchronized void start() {
			sLogcat.d("start daemon...");
						
			int resolution[] = { ScreenMetrics.getResolutionX(),
					ScreenMetrics.getResolutionY() };
			init(resolution);
			
			super.start();
		}
		
		@Override
		public void run() {
			sLogcat.d("run daemon...");
			doInBackground();
			super.run();
		}
		
		/**
		 * respond via {@link #ack()}
		 * 
		 * TODO: called back by JNI::Java_sync(), improvement needed!
		 */
		@SuppressLint("ack")
		public void sync() {			
			ScreenCoordsService.sLogcat.v("sync...");
			
			synchronized (mAvaiable) {
				mAvaiable = true;
			}
			
			if(mServiceRef.get() != null) {
				mServiceRef.get().notifyAllListeners();
				
				if(mAvaiable) {
					synchronized (this) {
						try {
							wait(TIME_OUT_MILLIS);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}
			
			ScreenCoordsService.sLogcat.v("sync ok.");
		}
		
		public void ack() {
			synchronized (mAvaiable) {
				mAvaiable = false;
			}
					
			synchronized (this) {
				notify();
			}
		}
		
		private synchronized void forceResetService(ScreenCoordsService service) {
			sDaemon.mServiceRef = new WeakReference<ScreenCoordsService>(service);
		}
		
		private NativeEventParserDaemon() {
			mServiceRef = new WeakReference<ScreenCoordsService>(null);
		}
		
		public native long getDownTime();
		public native long getEventTime();
		public native int getAction();
		public native float getPrecision(int axis);
		public native int getPointerCount();
		public native int[] getPointerIds();		
		public native PointerCoords[] getPointerCoords();
		public native void recycle();
		
		private native int init(int resolution[]);
		private native int doInBackground();
		
		static {
			System.loadLibrary("coords");
		}
	}

}
