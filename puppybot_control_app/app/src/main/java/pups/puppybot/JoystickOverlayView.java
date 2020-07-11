/**
 * 
 */
package pups.puppybot;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.OvalShape;
import android.util.AttributeSet;
import android.view.View;

/**
 * @author mpupilli
 */
public class JoystickOverlayView extends View {
	private int m_innerRadius;
	private int m_outerRadius;
	private int m_screenX;
	private int m_screenY;
	private int m_maxScreenRadius;
	
	ShapeDrawable m_boundary;
	ShapeDrawable m_hat;
	
	/**
	 * @param context
	 * @param attrs
	 */
	public JoystickOverlayView(Context context, AttributeSet attrs) {
		super(context, attrs);
		//if (!isInEditMode())
		//{
			TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.JoystickOverlayView);
			m_innerRadius = a.getInteger( R.styleable.JoystickOverlayView_innerRadius, 132 );
			m_outerRadius = a.getInteger( R.styleable.JoystickOverlayView_outerRadius, 150 );
			m_screenX = a.getInteger(R.styleable.JoystickOverlayView_positionX, 200);
			m_screenY = a.getInteger(R.styleable.JoystickOverlayView_positionY, 200);
			m_maxScreenRadius = a.getInteger(R.styleable.JoystickOverlayView_maxRadius, 150);
			int innerColour = a.getColor( R.styleable.JoystickOverlayView_innerColour, 0xDD2196F3 );
			int outerColour = a.getColor( R.styleable.JoystickOverlayView_outerColour, 0xAAFFFFFF );

			a.recycle();

			// Create the drawables:
			m_boundary = new ShapeDrawable( new OvalShape() );
			m_hat      = new ShapeDrawable( new OvalShape() );
			m_boundary.getPaint().setColor( outerColour );
			m_hat.getPaint().setColor( innerColour );
		//}
	}

	@Override public void onFinishInflate() {
		super.onFinishInflate();
		// NOTE: Can setup anything that depends on xml attributes here:
	}

    @Override
    protected void onDraw(Canvas canvas) {
    	m_boundary.draw( canvas );
    	m_hat.draw( canvas );
    }

    void SetJoystickCentre( int x, int y ) {
		m_boundary.setBounds( x-m_outerRadius, y-m_outerRadius, x+m_outerRadius, y+m_outerRadius );
    	SetJoystickPosition( x, y );
    }

    void SetJoystickPosition( int x, int y ) {
		m_hat.setBounds( x-m_innerRadius, y-m_innerRadius, x+m_innerRadius, y+m_innerRadius );
    	invalidate(); // Cause joystick to redraw when its position is changed.
    }

    int getScreenX() {
		return m_screenX;
	}

	int getScreenY() {
		return m_screenY;
	}

	int getMaxScreenRadius() {
		return m_maxScreenRadius;
	}
}
