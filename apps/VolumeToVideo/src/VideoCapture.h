//
// Created by wyz on 20-12-3.
//

#ifndef VOLUMETOVIDEO_VIDEOCAPTURE_H
#define VOLUMETOVIDEO_VIDEOCAPTURE_H


#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <string>


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>

#include <libavdevice/avdevice.h>

#include <libavfilter/avfilter.h>
//#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libavformat/avformat.h>
#include <libavformat/avio.h>

// libav resample

#include <libavutil/opt.h>
#include <libavutil/common.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/file.h>


// hwaccel
#include "libavcodec/vdpau.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vdpau.h"

// lib swresample

#include <libswscale/swscale.h>


extern std::ofstream logFile;

inline void Log(const std::string &str) {
    logFile.open("Logs.txt", std::ofstream::app);
    logFile.write(str.c_str(), str.size());
    logFile.close();
}

typedef void(*FuncPtr)(const char *);
extern FuncPtr ExtDebug;
extern char errbuf[32];

inline void Debug(std::string str, int err) {
    Log(str + " " + std::to_string(err));
    if (err < 0) {
        av_strerror(err, errbuf, sizeof(errbuf));
        str += errbuf;
    }
    Log(str);
    ExtDebug(str.c_str());
}

inline void avlog_cb(void *, int level, const char * fmt, va_list vargs) {
    std::string message;
//    vsnprintf_s(message, sizeof(message), fmt, vargs);

    Log(message);
}

class VideoCapture {
public:

    VideoCapture() {
        oformat = NULL;
        ofctx = NULL;
        videoStream = NULL;
        videoFrame = NULL;
        swsCtx = NULL;
        frameCounter = 0;

        // Initialize libavcodec
        av_register_all();
//        av_log_set_callback(avlog_cb);
    }

    ~VideoCapture() {
//        Free();
    }

    void Init(int width, int height, int fpsrate, int bitrate);

    void AddFrame(uint8_t *data);

    void Finish(const char* out_file_name);

private:

    AVOutputFormat *oformat;
    AVFormatContext *ofctx;

    AVStream *videoStream;
    AVFrame *videoFrame;

    AVCodec *codec;
    AVCodecContext *cctx;

    SwsContext *swsCtx;

    int frameCounter;

    int fps;

    void Free();


    void Remux(const char* out_file_name);
};

inline VideoCapture* Init(int width, int height, int fps, int bitrate) {
    VideoCapture *vc = new VideoCapture();
    vc->Init(width, height, fps, bitrate);
    return vc;
};

inline void AddFrame(uint8_t *data, VideoCapture *vc) {
    vc->AddFrame(data);
}

inline void Finish(VideoCapture *vc,const char* out_file_name) {
    vc->Finish(out_file_name);
}

inline void SetDebug(FuncPtr fp) {
    ExtDebug = fp;
};
}
#endif //VOLUMETOVIDEO_VIDEOCAPTURE_H
