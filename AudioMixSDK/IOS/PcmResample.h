#if !defined(__RESAMPLE_H_)
#define __RESAMPLE_H_
#include <pthread.h>

#define UINT unsigned int

class CResample  
{
//protected:
public:
	typedef struct {
		/* fractional resampling */
		UINT incr; /* fractional increment */
		UINT frac;
		int  last_sample;
		/* integer down sample */
		int  iratio;  /* integer divison ratio */
		int  icount, isum;
		int  inv;
	} ReSampleChannelContext;

	typedef struct ReSampleContext {
		ReSampleChannelContext channel_ctx[2];
		float ratio;
		/* channel convert */
		int input_channels, output_channels, filter_channels;
	} ReSampleContext;

	ReSampleContext	m_context;

protected:
	static void init_mono_resample(ReSampleChannelContext *s, float ratio);
	static int fractional_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);
	static int integer_downsample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);
	static void stereo_to_mono(short *output, short *input, int n1);
	static void mono_to_stereo(short *output, short *input, int n1);
	static void stereo_split(short *output1, short *output2, short *input, int n);
	static void stereo_mux(short *output, short *input1, short *input2, int n);
	static void ac3_5p1_mux(short *output, short *input1, short *input2, int n);
	static int mono_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);

public:
	CResample();
	virtual ~CResample();

	bool audio_resample_init(int output_channels, int input_channels, 
                             int output_rate, int input_rate);
	int audio_resample(short *output, short *input, int nb_samples);

	static int mono_8bit_to_16bit(short* lp16bits, unsigned char* lp8bits, int len);
	static int mono_16bit_to_8bit(unsigned char* lp8bits, short* lp16bits, int len);
};

class PcmQueue{
public:
    PcmQueue(unsigned long ulSampleInputSize);
    ~PcmQueue();
    unsigned long InsertData(char* pData, unsigned long ulDataSize);
    unsigned long GetData(char**ppData);
    unsigned long GetCurrentQueueLength();
    void CleanQueue();
private:
    PcmQueue();
    
private:
    unsigned long _ulSampleInputSize;
    char* _pSampleBuffer;
    unsigned long _ulSampleBufferSize;
    unsigned long _ulSampleBufferIndex;
    
    pthread_mutex_t _mConnstatMutex;
};

#endif // !defined(__RESAMPLE_H_)