package com.example.com.kaolafm.recordsdk;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import com.kaolafm.record.*;

public class MainActivity extends Activity {
    public AudioMixerNative _AudioMix;
    private String LOGMODULE="MainActivity";
    private Button _RecordStartButton;
    private Button _RecordStopButton;
    private Button _RecordCutButton;
    private AudioRecord     _AudioRecorder = null;
    private final int _iSampleRateDef = 32000;
    private final int _iBitRate       = 16000;
    private int _iRecorderBufferSize;
    private byte[] _RecorderBuffer;
    private boolean _mbStop;
    private HandlerThread _ProcessingThread;
    private Handler _AudioHandler;
    private int _FramePeriod;
    private FileOutputStream aacDataOutStream = null;
    private final String SAVE_FILENAME = "record_file.aac";
    private final String NEW_SAVE_FILENAME = "new_record_file.aac";
    
    private int AudioMixInit(){
    	int iRet = 0;
        _AudioMix = new AudioMixerNative();
        iRet = _AudioMix.PcmMixEncoderInit();
        Log.i(LOGMODULE, "PcmMixEncoderInit return "+iRet+"....");
        
        return iRet;
    }
    private void SetButtonEvent() {
		_RecordStartButton = (Button)findViewById(R.id.RecordStartButton);
		_RecordStopButton = (Button)findViewById(R.id.RecordStopButton);
		_RecordCutButton = (Button)findViewById(R.id.RecordCutButton);
		_mbStop = true;
		
		_RecordStartButton.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	if(!_mbStop){
            		return;
            	}
            	String strPath = Environment.getExternalStorageDirectory().getPath();
            	String filename = strPath+"/"+SAVE_FILENAME;
                File file = new File(filename);
                if(file.exists()){
                    file.delete();
                }
                try {
                    aacDataOutStream = new FileOutputStream(filename);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            	_mbStop = false;
            	_AudioRecorder.startRecording();
            	//_AudioRecorder.read(_RecorderBuffer, 0, _RecorderBuffer.length);
            }  
        }); 
		
		_RecordStopButton.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	if(_mbStop){
            		return;
            	}
            	_mbStop = true;
            	_AudioRecorder.stop();
            	try {
					aacDataOutStream.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
            	aacDataOutStream = null;
            }  
        });
		
		_RecordCutButton.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	if(!_mbStop){
            		return;
            	}
            	String strPath = Environment.getExternalStorageDirectory().getPath();
            	String filename = strPath+"/"+SAVE_FILENAME;
            	String outputFilename = strPath+"/"+NEW_SAVE_FILENAME;
            	
                File file = new File(outputFilename);
                if(file.exists()){
                    file.delete();
                }
            	_AudioMix.audioFileCut(filename, outputFilename, 5, 10, 64000);
            }  
        }); 
    }
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		_iRecorderBufferSize = AudioRecord.getMinBufferSize(_iSampleRateDef, 
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);
		_AudioRecorder = new AudioRecord(AudioSource.MIC, 
				_iSampleRateDef, AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,_iRecorderBufferSize);
		_RecorderBuffer = new byte[_iRecorderBufferSize];
		
		_ProcessingThread = new HandlerThread("AudioProcessing");
		_ProcessingThread.start();
		_AudioHandler = new Handler(_ProcessingThread.getLooper());
		
		int iChannelNum = 2;
		_FramePeriod = _iRecorderBufferSize / ( _iBitRate * iChannelNum / 8 );
		_AudioRecorder.setRecordPositionUpdateListener(updateListener, _AudioHandler);
		_AudioRecorder.setPositionNotificationPeriod(_FramePeriod);
		
		this.SetButtonEvent();
		this.AudioMixInit();
	}

	private AudioRecord.OnRecordPositionUpdateListener updateListener = new AudioRecord.OnRecordPositionUpdateListener()
	{
		public void onPeriodicNotification(AudioRecord recorder)
		{
			if(_mbStop){
				return;
			}
			int iPCMLen = _AudioRecorder.read(_RecorderBuffer, 0, _RecorderBuffer.length); // Fill buffer
			if(iPCMLen != _AudioRecorder.ERROR_BAD_VALUE){
				byte[] result = _AudioMix.MicPcmEncode(_iSampleRateDef, 2,
						_RecorderBuffer, iPCMLen);
				if(result != null){
					try {
						aacDataOutStream.write(result);
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
				        	
		}

		@Override
		public void onMarkerReached(AudioRecord arg0) {
			// TODO Auto-generated method stub
			
		}
	};
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
