#include "BitmapFrameReceiver.h"

/**
    @note This object holds a jobect reference to the bitmap for its entire lifetime.
*/
BitmapFrameReceiver::BitmapFrameReceiver( JavaVM* jvm, jobject jbitmap )
:
    m_jvm       ( jvm ),
    m_jbitmap	( 0 ),
    m_bitmapData( nullptr )
{
	JNIEnv* env;
	if ( m_jvm->GetEnv((void**)&env, JNI_VERSION_1_2) == JNI_OK )
	{
		m_jbitmap = env->NewGlobalRef(jbitmap);
	}
}

BitmapFrameReceiver::~BitmapFrameReceiver()
{
	JNIEnv* env;
	if ( m_jvm->GetEnv((void**)&env, JNI_VERSION_1_2) == JNI_OK )
	{
		env->DeleteGlobalRef(m_jbitmap);
	}
}

void BitmapFrameReceiver::LockReceiver()
{
	JNIEnv* env;
	if ( m_jvm->GetEnv((void**)&env, JNI_VERSION_1_2) == JNI_OK )
	{
		AndroidBitmap_getInfo( env, m_jbitmap, &m_bitmapInfo );
		AndroidBitmap_lockPixels( env, m_jbitmap, (void**)(&m_bitmapData) );
	}
}

void BitmapFrameReceiver::UnlockReceiver()
{
	JNIEnv* env;
	if ( m_jvm->GetEnv((void**)&env, JNI_VERSION_1_2) == JNI_OK )
	{
		AndroidBitmap_unlockPixels( env, m_jbitmap );
		m_bitmapData = 0;
	}
}

uint8_t* BitmapFrameReceiver::GetBuffer()
{
    return reinterpret_cast< uint8_t* >( m_bitmapData );
}

int BitmapFrameReceiver::GetStride()
{
    return m_bitmapInfo.stride;
}
