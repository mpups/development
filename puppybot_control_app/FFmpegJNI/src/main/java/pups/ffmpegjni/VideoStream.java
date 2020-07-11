package pups.ffmpegjni;

import android.graphics.Bitmap;

public class VideoStream {

	static {
		System.loadLibrary( "c++_shared" );
		System.loadLibrary( "avutil" );
		System.loadLibrary( "avcodec" );
		System.loadLibrary( "avformat" );
		System.loadLibrary( "swscale" );
		System.loadLibrary( "videolib" );
		System.loadLibrary( "robolib" );
		System.loadLibrary( "ffmpegjni" );
	}

	private native int createStream( String address, int port, int timeoutInSeconds );
	private native int getFrameWidth( int streamId );
	private native int getFrameHeight( int streamId );
	private native long getFrameCount( int streamId );
	private native double getStamp( int streamId );
	private native float getFrameRate( int streamId );
	private native void setFrameReceiver( int streamId, Bitmap bmp );
	private native boolean startStreaming( int streamId );
	private native void deleteStream( int streamId );
	private native int waitForFrame( int streamId );
	private native boolean isStreaming( int streamId );
	private native void sendJoystickUpdate( int streamId, int x, int y, int max );
	
	private int m_streamId = -1;
	
	/**
	 * Setup a VideoStream that will deliver frames to a Bitmap.
	 *  
	 * @param bmp a Bitmap in RGB_565 format allocated to the correct width and height to receive frames.
	 * 
	 * @throws Exception if the connection or stream could not be established, or if the bitmap is not the correct format.
	 * */
	public VideoStream( String address, int port, int timeoutInSeconds, Bitmap bmp ) throws Exception {
			
		if ( bmp == null || bmp.getConfig() != Bitmap.Config.RGB_565 ) {
			throw new Exception( "Bitmap must be allocated and have pixel format Bitmap.Config.RGB_565." );
		}
		
		m_streamId = createStream( address, port, timeoutInSeconds );
		if ( m_streamId < 0 ) {
			throw new Exception( "Could not create video stream." );
		}
		
		int w = getFrameWidth( m_streamId );
		int h = getFrameHeight( m_streamId );
		if ( bmp.getWidth() != w || bmp.getHeight() != h )
		{
			// @todo Currently the correct Bitmap size must magically be known by the caller.
			throw new Exception( "Bitmap is not correct size." );
		}
		
		setFrameReceiver( m_streamId, bmp ); // Must call this before startStreaming() as bitmap receiver interface requires access to the native streaming thread's JNIEnv.
		
		if ( startStreaming( m_streamId ) == false ) {
			throw new Exception( "Could not start the video streaming thread." );
		}
	}

	public void cleanup() {
		if ( m_streamId >= 0 ) {
			deleteStream( m_streamId );
		}
	}
	
	public int waitForFrame() {
		return waitForFrame( m_streamId );
	}
	
	public boolean isStreaming() {
		return isStreaming( m_streamId );
	}
	
	public void SendJoystickUpdate( int x, int y, int max )
	{
		sendJoystickUpdate( m_streamId, x, y, max );		
	}
	
	public double GetStamp()
	{
		return getStamp( m_streamId );
	}
	
	public float GetFrameRate()
	{
		return getFrameRate( m_streamId );
	}
}
