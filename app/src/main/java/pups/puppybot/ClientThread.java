package pups.puppybot;
import java.io.File;
import java.io.FileOutputStream;

import pups.ffmpegjni.VideoStream;
import android.graphics.Bitmap;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;

public class ClientThread extends Thread {
   	private volatile boolean m_go;
	private volatile boolean m_updateImage = false;
	private Bitmap m_bitmap;
	private Handler m_handler;
	private String m_ipAddress;
	private int m_portNumber;
	private VideoStream m_stream;
	private double m_stamp;
	
    private void parseIpAddress( final String addrString ) {        
        // split into IP address and port number:
    	
        try {
            int i = addrString.indexOf( ':' );
        	if ( i >= 0 )
        	{
        		String portString = new String( addrString.substring( i+1 ) ); 
        		m_portNumber = Integer.parseInt( portString );
        		m_ipAddress = new String( addrString.substring( 0, i ) );
        	}
        }
        catch ( Exception e ) {
        	Log.e( "ClientThread", "Could not interpret the IP address.", e );
        	m_portNumber = 0;
        }
    }
	
	ClientThread( Handler handler, String addressEntry ) {
		m_handler = handler;
		parseIpAddress( addressEntry );
	}
	
	void terminate() {
		m_go = false;
	}
	
	public Bitmap GetBitmap() throws Exception {
		if ( m_bitmap != null )
		{
			return m_bitmap;   			
		}
		throw new Exception( "Bitmap is null" );
	}

	public double GetStamp()
	{
		return m_stamp;
	}
	
	public float GetFps() {
		return m_stream.GetFrameRate();
	}
	
	void enableImageUpdates() {
		m_updateImage = true;
	}

	void disableImageUpdates() {
		m_updateImage = false;
	}
	
	void saveImage() {
		
		if ( Environment.getExternalStorageState().equals( Environment.MEDIA_MOUNTED ) )
		{
			try {
				String fileName = Environment.getExternalStorageDirectory().getName()
								+ File.separator + "robot_capture.png";
				File file = new File( fileName );
				Log.i( "ClientThread", "Saving image: " + file );
				FileOutputStream out = new FileOutputStream( file );
				m_bitmap.compress( Bitmap.CompressFormat.PNG, 100, out );
				out.close();
			}
			catch ( Exception e) {
				Log.e( "ClientThread", "Exception: ", e);
			}
		}
		else
		{
			Log.w( "ClientThread", "File not saved: Media not available.");
		}
	}
	
	// Send joystick command back over video stream's socket.
	void sendJoystickControl( int jx, int jy, int jmax )
	{
		if ( m_stream != null )
		{
			m_stream.SendJoystickUpdate( jx, jy, jmax );
		}
    }
	
	@Override
	public void run() {
		m_go = true;
		
		try {
			Log.i( "Puppybot:ClientThread", "New client started" );
			
			m_bitmap = Bitmap.createBitmap( 320, 240, Bitmap.Config.RGB_565 );
	        
			m_handler.sendEmptyMessage( ClientState.CONNECTING );
			final int timeoutInSeconds = 5;
			m_stream = new VideoStream( m_ipAddress, m_portNumber, timeoutInSeconds, m_bitmap );
			m_handler.sendEmptyMessage( ClientState.CONNECTED );
			
			int lastFrame = m_stream.waitForFrame(); // Must call this before looping so that the frame number is valid AND we do not
										      // test IsStreaming() before the streaming thread has actually started.
	        while ( m_go && m_stream.isStreaming() )
	        {        	
	        	int frameNumber = m_stream.waitForFrame();
	        	if ( m_updateImage && frameNumber > lastFrame ) {
	        		m_stamp = m_stream.GetStamp();
	        		m_handler.sendEmptyMessage( ClientState.NEWIMAGE );
	        		lastFrame = frameNumber;
	        	}
	        }
	        
	        Log.i( "Puppybot:ClientThread", "Client thread finished" );
	        m_stream.cleanup();
	        m_stream = null;
	        
			m_handler.sendEmptyMessage( ClientState.DISCONNECTED );
		}
		catch ( Exception e )
		{
			Log.e( "ClientThread", "Exception: ", e);
			m_handler.sendEmptyMessage( ClientState.CON_FAILED );
		}
	}	
}
