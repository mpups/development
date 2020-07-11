/**
 * 
 */
package pups.puppybot;

import android.os.Vibrator;
import android.view.MotionEvent;
import android.content.Context;

/**
 * @author mpupilli
 *
 * Encapsulates processing for using the touch screen as a Joystick control.
 *
 */
public class TouchJoystick {
	private int m_cx;
	private int m_cy;
	private int m_jx;
	private int m_jy;
	private int m_jmax;
	private boolean m_multiTouchActive;
	private boolean m_inDeadZone; // true if we are in dead zone in horizontal-axis (used for tactile feedback).
	
	public TouchJoystick( int centreX, int centreY, int maxRadius ) {
		m_cx = centreX;
		m_cy = centreY;
		m_jx = 0;
		m_jy = 0;
		m_jmax = maxRadius;
		m_multiTouchActive = false;
		m_inDeadZone = true;
	}

	public int getX() {
		return m_jx;
	}
	
	public int getY() {
		return m_jy;
	}
	
	public int getCentreX() {
		return m_cx;
	}
	
	public int getCentreY() {
		return m_cy;
	}
	
	public int getMax() {
		return m_jmax;
	}
	
	/**
	 * Processes touch event and stores the joystick data internally.
	 * The resulting joystick position can be accessed with the getX() and getY() methods.
	 * 
	 * The joystick also gives tactile feedback to help the user centre the joystick and drive straight.
	 * 
	 * @param e motion event received by the activity to be processed as joystick input
	 * @param context the main Activity's context - used to access the device's VIBRATE_SERVICE.
	 * **/
	public boolean processTouchEvent( MotionEvent e, Context context )
	{
		// First: in case of multi-touch we centre joystick to
		// disable motion because multi-touch is unreliable on some phones.
		if ( e.getPointerCount() > 1 ) {
			m_jx = 0;
			m_jy = 0;
			return true;
		}
		
		boolean consumed = false;

		int action = e.getAction();
		
		if ( action == MotionEvent.ACTION_UP ) {
			// When touch is released released we automatically centre the joystick:
			m_jx = 0;
			m_jy = 0;
			consumed = true;
			m_inDeadZone = true;
		}
		
		if ( action == MotionEvent.ACTION_MOVE && m_multiTouchActive == false ) {
			float jx = e.getX() - m_cx;
			float jy = e.getY() - m_cy;
			
			if ( Math.abs(jx) > m_jmax ) {
				jx = Math.signum( m_jx )*m_jmax;
			}
			if ( Math.abs(jy) > m_jmax )
			{
				jy = Math.signum( jy )*m_jmax;
			}
			
			// apply dead zone:
			boolean nowInDeadZone = false;
			if ( Math.abs(jx) < (m_jmax*.1f) )
			{
				jx = 0.f;
				nowInDeadZone = true;
			}
			if ( Math.abs(jy) < (m_jmax*.1f) )
			{
				jy = 0.f;
			}
			
			if ( (!m_inDeadZone && nowInDeadZone) || (m_inDeadZone && !nowInDeadZone) ) {
				// Vibrate when the joystick enters/leaves the deadzone in horizontal-axis
				// (axis which controls turn rate). This gives tactile feedback to the user
				// to help them drive the robot straight (i.e. they can centre the joystick
				// without looking at the screen).
				Vibrator v = (Vibrator) context.getSystemService( Context.VIBRATOR_SERVICE );
				if ( v != null ) {
					v.vibrate(50);
				}
			}
			
			m_inDeadZone = nowInDeadZone;
			
			m_jx = (int)jx;
			m_jy = (int)jy;
			
			consumed = true;
		}
		
		return consumed;
	}

	/**
	 * Zero the joystick position.
	 * */
	public void zero() {
		m_jx = 0;
		m_jy = 0;
	}
	
}
