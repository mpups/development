package pups.puppybot;

import pups.puppybot.JoystickOverlayView;
import pups.puppybot.TouchJoystick;
import pups.puppybot.ClientState;

import java.io.IOException;
import java.io.InputStream;
import java.util.Formatter;
import java.util.Timer;
import java.util.TimerTask;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Vibrator;

import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class PuppyBotControl extends Activity implements Handler.Callback {
	private ProgressBar m_bar;
	private ClientThread m_client;
	private TouchJoystick m_joy;
	private JoystickOverlayView m_joyView;
	private Timer m_timerThread = new Timer();
	private boolean m_controlEnabled = false;
	private StringBuilder m_infoTextBuilder = new StringBuilder(128);
	
    /** Called when the activity is first created. */
	@Override
    public void onCreate( Bundle savedInstanceState ) {
        super.onCreate( savedInstanceState );
        
        // Make view full-screen:
        getWindow().requestFeature( Window.FEATURE_NO_TITLE );
        getWindow().addFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN );
               
        setContentView( R.layout.main );

        m_bar = (ProgressBar) findViewById( R.id.connectionProgress );
        m_bar.setVisibility( View.INVISIBLE );

		m_joyView = (JoystickOverlayView)findViewById(R.id.joystickOverlayView);
		int joyRadius = m_joyView.getMaxScreenRadius();
		int joyX = m_joyView.getScreenX();
		int joyY = m_joyView.getScreenY();
        m_joy = new TouchJoystick(joyX, joyY, joyRadius);
        m_joyView.SetJoystickCentre(m_joy.getCentreX(), m_joy.getCentreY());

        // Set the IP address to the last successful connection, or the default if never connected:
        EditText addrEntry = (EditText) findViewById( R.id.IpAddressEntry );
        SharedPreferences pref = getPreferences( Activity.MODE_PRIVATE );
        addrEntry.setText( pref.getString( "last_successful_connection", getString( R.string.default_address ) ) );
        
        InputStream inputStream;
		try {
			inputStream = getAssets().open( "splash.png" );
			Bitmap startImage = BitmapFactory.decodeStream( inputStream, null, null );
			ImageView imgView = (ImageView) findViewById( R.id.cameraView );
			imgView.setImageBitmap( startImage );
		} catch (IOException e) {
			Log.e( "PuppyBotControl", "Could not load splash screen asset." );
			e.printStackTrace();
		}
		
		scheduleJoystickCommands();
    }

	private void scheduleJoystickCommands() {
		// Create a task that will send joystick commands at 15Hz:
		TimerTask joyTask = new TimerTask() {
			public void run()
			{
				if ( m_client != null && m_controlEnabled )
				{
					m_client.sendJoystickControl( m_joy.getY(), m_joy.getX(), m_joy.getMax() );
				}
			}
		};
		int hz = 15;
		int delay_ms = 500;
		int period_ms = 1000/hz;
		m_timerThread.schedule( joyTask, delay_ms, period_ms );
	}

    @Override
    protected void onStart() {
        super.onStart();
               
        // The activity is about to become visible.
        if ( m_client != null ) {
        	m_client.enableImageUpdates();
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        // The activity has become visible (it is now "resumed").
        
        m_joyView.invalidate();
        
        // Let's check the Wi-Fi state - we only really ever want to connect to a robot via Wi-Fi:
        try {
            ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService( Context.CONNECTIVITY_SERVICE );
        	NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
            if ( networkInfo != null )
            {
    			ImageView imgView = (ImageView) findViewById( R.id.wifiStatus );
            	if( networkInfo.getType() == ConnectivityManager.TYPE_WIFI )
            	{
            		Log.i( "PuppyBotControl", "Wi-Fi active." );
        			imgView.setImageResource( R.drawable.wifi_state_on );
            	}
            	else
            	{
            		Log.i( "PuppyBotControl", "No Wi-Fi connection." );
            		Toast.makeText(getApplicationContext(), "You probably want to enable Wi-Fi.", Toast.LENGTH_LONG).show();
        			imgView.setImageResource( R.drawable.wifi_state_off );
            	}
            }
            
            m_controlEnabled = true;
        	
        } catch ( Exception e ) {
        	Log.e( "PuppyBotControl", "Can't access Wi-Fi state." );
        }
    }
    
    @Override
    protected void onPause() {
        // Another activity is taking focus (this activity is about to be "paused").
        
        /// Zero the joystick controls so that robot does not run away if app is interrupted (e.g. by phone call)
        m_controlEnabled = false;
        m_joy.zero();
        m_joyView.SetJoystickPosition( m_joy.getCentreX(), m_joy.getCentreY() );
        
        super.onPause();
    }
    
    @Override
    protected void onStop() {       
        // The activity is no longer visible (it is now "stopped")
        if ( m_client != null ) {
        	m_client.disableImageUpdates();
        }
        
        super.onStop();
    }
    
    @Override
    protected void onDestroy() {
        
    	indicateDisconnected();
        
        // The activity is about to be destroyed so try to stop the client thread.
        if ( m_client != null ) {
        	try {
        		m_client.terminate();
        		m_client.join(); // no timeout - Android will kill us anyway
        	} catch (Exception e) {
        		// Don't care
        	}
        }
        
        super.onDestroy();
    }
    
	public boolean onTouchEvent( MotionEvent e )
	{
		boolean consumed = m_joy.processTouchEvent( e, getApplicationContext() );
		
		if ( m_controlEnabled && consumed )
		{
			int x = m_joy.getX() + m_joy.getCentreX();
			int y = m_joy.getY() + m_joy.getCentreY();
		
			m_joyView.SetJoystickPosition( x, y );
		}
	
		return consumed;
	}
    
	private String getIpAddress() {
		// get the ip address entry text:
		return ((EditText) findViewById( R.id.IpAddressEntry )).getText().toString();
	}
	   
    public void toggleListener( View v ) {
    	if ( m_client != null && m_client.isAlive() ) {
    		// Client still running so send request to terminate to client thread.
    		m_client.terminate();
    	}
    	else if ( ((ToggleButton)v).isChecked() ) {
            m_client = new ClientThread( new Handler( this ), getIpAddress() );
            m_client.enableImageUpdates();
    		m_client.start();
    	}
    }
    
	@TargetApi(Build.VERSION_CODES.KITKAT)
	@Override
    public boolean handleMessage( Message m ) {
    	switch ( m.what ) {
    	case ClientState.CONNECTING:
    		indicateConnecting();
    		break;
    	case ClientState.CONNECTED:
    		indicateConnected();
            View rootView = getWindow().getDecorView();
            if (rootView != null) {
    	    	if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT ) {
    	    		rootView.setSystemUiVisibility( View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION );
    	    	}
        	}
    		break;
    		
    	case ClientState.CON_FAILED:
        	Toast.makeText(getApplicationContext(), "Connection Failed", Toast.LENGTH_LONG).show();
        	// Fall through to next case so that GUI indicates disconnection:
    	case ClientState.DISCONNECTED:
    		indicateDisconnected();
    		break;
    		
    	case ClientState.NEWIMAGE:
    		try
    		{
    			ImageView imgView = (ImageView) findViewById( R.id.cameraView );
    			imgView.setImageBitmap( m_client.GetBitmap() );
    			TextView stampView = (TextView) findViewById( R.id.stampText );

    			// Build the info string:
    			m_infoTextBuilder.setLength(0);
    			Formatter fmt = new Formatter(m_infoTextBuilder);
    			fmt.format("stamp: %.2f\n%.1f fps", m_client.GetStamp(), m_client.GetFps() );
    			stampView.setText( m_infoTextBuilder.toString() );
    		}
    		catch ( Exception e )
    		{
    			Log.e( "PuppyBotControl", e.toString() );
    		}
    		break;
    	}
    	
    	return true;
    }

    private void indicateConnecting() {
    	TextView stampView = (TextView) findViewById( R.id.stampText );
    	stampView.setText("");
    	
    	m_bar.setVisibility( View.VISIBLE );
    	ToggleButton button = (ToggleButton)findViewById( R.id.connectToggleButton );
    	button.setText( "Connecting..." );
    	button.setEnabled( false );
    	
		EditText addrEntry = (EditText) findViewById( R.id.IpAddressEntry );
		addrEntry.setEnabled( false );
		// disable editing, you would think the above would
		// do this but no, you have to do all of the following *sigh*:
		addrEntry.setClickable( false );
		addrEntry.setFocusable( false );
		addrEntry.setFocusableInTouchMode( false );
    }

    private void indicateConnected() {
    	m_bar.setVisibility( View.INVISIBLE );
    	ToggleButton button = (ToggleButton)findViewById( R.id.connectToggleButton );
    	button.setText( "Connected" );
    	button.setEnabled( true );
    	
    	EditText addrEntry = (EditText) findViewById( R.id.IpAddressEntry );
		addrEntry.setVisibility( View.INVISIBLE );
		
		SharedPreferences.Editor prefEdit = getPreferences( Activity.MODE_PRIVATE ).edit();
		prefEdit.putString( "last_successful_connection", addrEntry.getText().toString() );
		prefEdit.commit();
		
		Toast.makeText(getApplicationContext(), "Connected Successfully", Toast.LENGTH_LONG).show();
		
		Vibrator v = (Vibrator) getSystemService( VIBRATOR_SERVICE );
		if ( v != null ) {
			v.vibrate(50);
		}
    }
    
    private void indicateDisconnected() {
    	m_bar.setVisibility( View.INVISIBLE );
    	ToggleButton button = (ToggleButton)findViewById( R.id.connectToggleButton );
    	button.setChecked( false );
    	button.setEnabled( true );
    	button.setText( "Disconnected" );
    	
    	Toast.makeText(getApplicationContext(), "Disconnected", Toast.LENGTH_LONG).show();
    	
    	EditText addrEntry = (EditText) findViewById( R.id.IpAddressEntry );
		addrEntry.setVisibility( View.VISIBLE );
		addrEntry.setEnabled( true );
		// enable editing:
		addrEntry.setClickable( true );
		addrEntry.setFocusable( true );
		addrEntry.setFocusableInTouchMode( true );
    }
}
