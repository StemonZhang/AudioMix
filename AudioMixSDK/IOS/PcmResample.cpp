#include "PcmResample.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FRAC_BITS		16
#define FRAC			(1 << FRAC_BITS)
#define av_free(p)		{if(p) free(p);}
#define av_malloc(size)	malloc(size)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResample::CResample()
{
}

CResample::~CResample()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 8bit_to_16bit(◊¢“‚: ‰≥ˆ ˝æ› «”–∑˚∫≈µƒªπ «Œﬁ∑˚∫≈µƒ)
// output:   para 1: 16bitµƒ ˝æ›
// input:    para 2: 8bit ˝æ›       
//           para 3: “™◊™ªª ˝æ›µƒ¥Œ ˝£¨Œ™‘≠ ˝æ›≥§∂»£¨

//◊¢:“ÚŒ™“ª¥Œ◊™ªª1∏ˆ◊÷Ω⁄±‰¡Ω∏ˆ◊÷Ω⁄£¨À˘“‘◊™ªª∫Û ˝æ›µƒ◊‹≥§∂»Œ™8Œª ˝æ›µƒ¡Ω±∂
int CResample::mono_8bit_to_16bit(short* lp16bits, unsigned char* lp8bits, int len)
{
	int i=0;
	for(i=0; i<len; i++) {
		*lp16bits++ = ((*lp8bits++) -128) << 8;
	}   

	return i<<1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CResample::init_mono_resample(ReSampleChannelContext *s, float ratio)
{
    ratio = (float)(1.0 / ratio);
    s->iratio = (int)floorf(ratio);
    if (s->iratio == 0)
        s->iratio = 1;
    s->incr = (int)((ratio / s->iratio) * FRAC);
    s->frac = FRAC;
    s->last_sample = 0;
    s->icount = s->iratio;
    s->isum = 0;
    s->inv = (FRAC / s->iratio);
}

/* fractional audio resampling */
int CResample::fractional_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples)
{
    unsigned int frac, incr;
    int l0, l1;
    short *q, *p, *pend;

    l0 = s->last_sample;
    incr = s->incr;
    frac = s->frac;

    p = input;
    pend = input + nb_samples;
    q = output;

    l1 = *p++;
    for(;;) {
        /* interpolate */
        *q++ = (l0 * (FRAC - frac) + l1 * frac) >> FRAC_BITS;
        frac = frac + s->incr;
        while (frac >= FRAC) {
            frac -= FRAC;
            if (p >= pend)
                goto the_end;
            l0 = l1;

			l1 = *p++;
        }
    }
 the_end:
    s->last_sample = l1;
    s->frac = frac;
    return q - output;
}

int CResample::integer_downsample(ReSampleChannelContext *s, short *output, short *input, int nb_samples)
{
    short *q, *p, *pend;
    int c, sum;

    p = input;
    pend = input + nb_samples;
    q = output;

    c = s->icount;
    sum = s->isum;

    for(;;) {
        sum += *p++;
        if (--c == 0) {
            *q++ = (sum * s->inv) >> FRAC_BITS;
            c = s->iratio;
            sum = 0;
        }
        if (p >= pend)
            break;
    }
    s->isum = sum;
    s->icount = c;
    return q - output;
}

/* n1: number of samples */
void CResample::stereo_to_mono(short *output, short *input, int n1)
{
    short *p, *q;
    int n = n1;

    p = input;
    q = output;
    while (n >= 4) {
        q[0] = (p[0] + p[1]) >> 1;
        q[1] = (p[2] + p[3]) >> 1;
        q[2] = (p[4] + p[5]) >> 1;
        q[3] = (p[6] + p[7]) >> 1;
        q += 4;
        p += 8;
        n -= 4;
    }
    while (n > 0) {
        q[0] = (p[0] + p[1]) >> 1;
        q++;
        p += 2;
        n--;
    }
}

/* n1: number of samples */
void CResample::mono_to_stereo(short *output, short *input, int n1)
{
    short *p, *q;
    int n = n1;
    int v;

    p = input;
    q = output;
    while (n >= 4) {
        v = p[0]; q[0] = v; q[1] = v;
        v = p[1]; q[2] = v; q[3] = v;
        v = p[2]; q[4] = v; q[5] = v;
        v = p[3]; q[6] = v; q[7] = v;
        q += 8;
        p += 4;
        n -= 4;
    }
    while (n > 0) {
        v = p[0]; q[0] = v; q[1] = v;
        q += 2;
        p += 1;
        n--;
    }
}

/* XXX: should use more abstract 'N' channels system */
void CResample::stereo_split(short *output1, short *output2, short *input, int n)
{
    int i;

    for(i=0;i<n;i++) {
        *output1++ = *input++;
        *output2++ = *input++;
    }
}

void CResample::stereo_mux(short *output, short *input1, short *input2, int n)
{
    int i;

    for(i=0;i<n;i++) {
        *output++ = *input1++;
        *output++ = *input2++;
    }
}

void CResample::ac3_5p1_mux(short *output, short *input1, short *input2, int n)
{
    int i;
    short l,r;

    for(i=0;i<n;i++) {
      l=*input1++;
      r=*input2++;
      *output++ = l;           /* left */
      *output++ = (l/2)+(r/2); /* center */
      *output++ = r;           /* right */
      *output++ = 0;           /* left surround */
      *output++ = 0;           /* right surroud */
      *output++ = 0;           /* low freq */
    }
}

int CResample::mono_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples)
{
    short *buf1;
    short *buftmp;

    buf1= (short*)av_malloc( nb_samples * sizeof(short) );

    /* first downsample by an integer factor with averaging filter */
    if (s->iratio > 1) {
        buftmp = buf1;
        nb_samples = integer_downsample(s, buftmp, input, nb_samples);
    } else {
        buftmp = input;
    }

    /* then do a fractional resampling with linear interpolation */
    if (s->incr != FRAC) {
        nb_samples = fractional_resample(s, output, buftmp, nb_samples);
    } else {
        memcpy(output, buftmp, nb_samples * sizeof(short));
    }
    av_free(buf1);
    return nb_samples;
}

bool CResample::audio_resample_init(int output_channels, int input_channels, 
                                    int output_rate, int input_rate)
{
    int i;
    
    if ( input_channels > 2)
    {
		printf("Resampling with input channels greater than 2 unsupported.");
		return false;
    }

	memset(&m_context, 0, sizeof(ReSampleContext));

    m_context.ratio = (float)output_rate / (float)input_rate;
    
    m_context.input_channels = input_channels;
    m_context.output_channels = output_channels;
    
    m_context.filter_channels = m_context.input_channels;
    if (m_context.output_channels < m_context.filter_channels)
        m_context.filter_channels = m_context.output_channels;

/*
 * ac3 output is the only case where filter_channels could be greater than 2.
 * input channels can't be greater than 2, so resample the 2 channels and then
 * expand to 6 channels after the resampling.
 */
    if(m_context.filter_channels>2)
      m_context.filter_channels = 2;

    for(i=0;i<m_context.filter_channels;i++) {
        init_mono_resample(&m_context.channel_ctx[i], m_context.ratio);
    }
    return true;
}

/* resample audio. 'nb_samples' is the number of input samples */
/* XXX: optimize it ! */
/* XXX: do it with polyphase filters, since the quality here is
   HORRIBLE. Return the number of samples available in output */
// ÷ÿ≤…—˘
// output para1:÷ÿ≤…—˘∫Û ‰≥ˆµƒ ˝æ›
// input  para2: ‰»Î ˝æ›
//        para3:¥À÷°“Ù∆µµƒ≤…—˘µ„ ˝
// ◊¢:44100µƒ≤…—˘µ„ ˝πÃ∂®Œ™1024£¨∆‰À˚≤…—˘¬ ≤ªπÃ∂®£¨–Ë“™Õ®µ¿¥À÷°≥§∂»£¨Õ®µ¿ ˝£¨ŒªøÌ£¨º∆À„≤…—˘µ„ ˝
int CResample::audio_resample(short *output, short *input, int nb_samples)
{
    int i, nb_samples1;
    short *bufin[2];
    short *bufout[2];
    short *buftmp2[2], *buftmp3[2];
    int lenout;

    if (m_context.input_channels == m_context.output_channels && m_context.ratio == 1.0) {
        /* nothing to do */
        memcpy(output, input, nb_samples * m_context.input_channels * sizeof(short));
        return nb_samples;
    }

    /* XXX: move those malloc to resample init code */
    bufin[0]= (short*) av_malloc( nb_samples * sizeof(short) );
    bufin[1]= (short*) av_malloc( nb_samples * sizeof(short) );
    
    /* make some zoom to avoid round pb */
    lenout= (int)(nb_samples * m_context.ratio) + 16;
    bufout[0]= (short*) av_malloc( lenout * sizeof(short) );
    bufout[1]= (short*) av_malloc( lenout * sizeof(short) );

    if (m_context.input_channels == 2 &&
        m_context.output_channels == 1) {
        buftmp2[0] = bufin[0];
        buftmp3[0] = output;
        stereo_to_mono(buftmp2[0], input, nb_samples);
    } else if (m_context.output_channels >= 2 && m_context.input_channels == 1) {
        buftmp2[0] = input;
        buftmp3[0] = bufout[0];
    } else if (m_context.output_channels >= 2) {
        buftmp2[0] = bufin[0];
        buftmp2[1] = bufin[1];
        buftmp3[0] = bufout[0];
        buftmp3[1] = bufout[1];
        stereo_split(buftmp2[0], buftmp2[1], input, nb_samples);
    } else {
        buftmp2[0] = input;
        buftmp3[0] = output;
    }

    /* resample each channel */
    nb_samples1 = 0; /* avoid warning */
    for(i=0;i<m_context.filter_channels;i++) {
        nb_samples1 = mono_resample(&m_context.channel_ctx[i], buftmp3[i], buftmp2[i], nb_samples);
    }

    if (m_context.output_channels == 2 && m_context.input_channels == 1) {
        mono_to_stereo(output, buftmp3[0], nb_samples1);
    } else if (m_context.output_channels == 2) {
        stereo_mux(output, buftmp3[0], buftmp3[1], nb_samples1);
    } else if (m_context.output_channels == 6) {
        ac3_5p1_mux(output, buftmp3[0], buftmp3[1], nb_samples1);
    }

    av_free(bufin[0]);
    av_free(bufin[1]);

    av_free(bufout[0]);
    av_free(bufout[1]);
    return nb_samples1;
}


// 16bit_to_8bit(◊¢“‚ ‰≥ˆ ˝æ› «”–∑˚∫≈µƒªπ «Œﬁ∑˚∫≈µƒ)
// output:   para 1: 8bitµƒ ˝æ›
// input:    para 2: 16bit ˝æ›       
//           para 3: “™◊™ªª ˝æ›µƒ¥Œ ˝£¨Œ™‘≠ ˝æ›≥§∂»µƒ“ª∞Î£¨“ÚŒ™“ª¥Œ◊™ªª2∏ˆ◊÷Ω⁄
int CResample::mono_16bit_to_8bit(unsigned char* lp8bits, short* lp16bits, int len)
{
	int i=0;
	for(i=0; i<len; i++) {
		*lp8bits++ = ((*lp16bits++) >> 8) + 128;
	}   
	
	return i>>1;
}

PcmQueue::PcmQueue(unsigned long ulSampleInputSize){
    _ulSampleInputSize = ulSampleInputSize;
    _ulSampleBufferSize = _ulSampleInputSize*sizeof(short)*2;
    _pSampleBuffer = (char*)malloc(_ulSampleBufferSize);
    _ulSampleBufferIndex = 0;
    
    pthread_mutex_init(&_mConnstatMutex,NULL);
}

PcmQueue::~PcmQueue(){
    free((void*)_pSampleBuffer);
}

unsigned long PcmQueue::InsertData(char* pInputData, unsigned long ulDataSize)
{
    pthread_mutex_lock(&_mConnstatMutex);

    if (_ulSampleBufferIndex == 0) {
        if (ulDataSize >= _ulSampleBufferSize){
            free(_pSampleBuffer);
            _ulSampleBufferSize = ulDataSize*2;
            _pSampleBuffer = (char*)malloc(_ulSampleBufferSize);
            memcpy((void*)_pSampleBuffer, (void*)pInputData, ulDataSize);
            _ulSampleBufferIndex = ulDataSize;
        }else{
            memcpy((void*)(_pSampleBuffer+_ulSampleBufferIndex), (void*)pInputData, ulDataSize);
            _ulSampleBufferIndex = ulDataSize;
        }
    }else{
        if ((_ulSampleBufferIndex+ulDataSize)>=_ulSampleBufferSize) {
            char* pNewBuffer = (char*)malloc(_ulSampleBufferIndex+ulDataSize);
            
            memcpy((void*)pNewBuffer, (void*)_pSampleBuffer, _ulSampleBufferIndex);
            memcpy((void*)(pNewBuffer+_ulSampleBufferIndex), pInputData, ulDataSize);
            _ulSampleBufferSize = _ulSampleBufferIndex+ulDataSize;
            free(_pSampleBuffer);
            _pSampleBuffer = pNewBuffer;
            _ulSampleBufferIndex = _ulSampleBufferIndex + ulDataSize;
        }else{
            memcpy((void*)(_pSampleBuffer+_ulSampleBufferIndex), (void*)pInputData, ulDataSize);
            _ulSampleBufferIndex += ulDataSize;
        }
    }
    pthread_mutex_unlock(&_mConnstatMutex);
    return _ulSampleBufferIndex;
}

unsigned long PcmQueue::GetCurrentQueueLength(){
    pthread_mutex_lock(&_mConnstatMutex);
    unsigned long ulRet = _ulSampleBufferIndex;
    pthread_mutex_unlock(&_mConnstatMutex);
    return ulRet;
}

void PcmQueue::CleanQueue(){
    pthread_mutex_lock(&_mConnstatMutex);
    _ulSampleBufferIndex = 0;
    pthread_mutex_unlock(&_mConnstatMutex);
}

unsigned long PcmQueue::GetData(char** ppData){
    const unsigned long ulExpectedLength = _ulSampleInputSize*sizeof(short);
    unsigned long ulRet = 0;
    
    pthread_mutex_lock(&_mConnstatMutex);
    if (_ulSampleBufferIndex < ulExpectedLength) {
        ulRet = 0;
        *ppData = NULL;
    }else if (_ulSampleBufferIndex > ulExpectedLength) {
        char* pOutputData = (char*)malloc(ulExpectedLength);
        memcpy(pOutputData, _pSampleBuffer, ulExpectedLength);
        *ppData = pOutputData;
        
        char* pNewDataBuffer = (char*)malloc(_ulSampleBufferSize);
        memcpy((void*)pNewDataBuffer, (void*)(_pSampleBuffer+ulExpectedLength), (_ulSampleBufferIndex-ulExpectedLength));
        _ulSampleBufferIndex = _ulSampleBufferIndex-ulExpectedLength;
        free(_pSampleBuffer);
        _pSampleBuffer = pNewDataBuffer;
        ulRet = ulExpectedLength;
    }else{
        char* pOutputData = (char*)malloc(ulExpectedLength);
        memcpy(pOutputData, _pSampleBuffer, ulExpectedLength);
        *ppData = pOutputData;
        _ulSampleBufferIndex = 0;
        ulRet = ulExpectedLength;
    }

    pthread_mutex_unlock(&_mConnstatMutex);
    return ulRet;
}
