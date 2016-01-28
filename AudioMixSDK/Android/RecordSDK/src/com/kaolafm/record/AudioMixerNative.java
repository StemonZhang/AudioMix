package com.kaolafm.record;

//javah -classpath bin/classes -d jni com.kaolafm.record.AudioMixerNative

public class AudioMixerNative {
	public native int PcmMixEncoderInit();
	public native void PcmMixEncoderDeInit();
	public native byte[] MusicPcmMixEncode(int iSampleRate, int iChannelNumber, 
			byte[] pData, int iLen);
	public native byte[] MicPcmMixEncode(int iSampleRate, int iChannelNumber, 
			byte[] pData, int iLen);
    public native byte[] MusicPcmEncode(int iSampleRate, int iChannelNumber, 
    		byte[] pData, int iLen);
    public native byte[] MicPcmEncode(int iSampleRate, int iChannelNumber, 
    		byte[] pData, int iLen);
    public native byte[] PcmMixFlush();
    public native void MusicGain(float fMusicGain);
    public native void MicGain(float fMicGain);
    
    public native int audioFileCut(String InputFilePathname, String OutputFilePathname, 
    		float fStartTime, float fEndTime, float fBitRate);
    static {
        System.loadLibrary("recordsdk");  
    }
}