//
// Created by wyz on 20-12-3.
//
#include "VideoCapture.h"

#define VIDEO_TMP_FILE "tmp.h264"
#define FINAL_FILE_NAME "record.mp4"

using namespace std;

std::ofstream logFile;
FuncPtr ExtDebug;
char errbuf[32];

void VideoCapture::Init(int width, int height, int fpsrate, int bitrate) {
  fps = fpsrate;

  int err;

  if (!(oformat = av_guess_format(NULL, VIDEO_TMP_FILE, NULL))) {
    Debug("Failed to define output format", 0);
    return;
  }

  if ((err = avformat_alloc_output_context2(&ofctx, oformat, NULL,
                                            VIDEO_TMP_FILE) < 0)) {
    Debug("Failed to allocate output context", err);
    Free();
    return;
  }

  if (!(codec = avcodec_find_encoder(oformat->video_codec))) {
    Debug("Failed to find encoder", 0);
    Free();
    return;
  }

  if (!(videoStream = avformat_new_stream(ofctx, codec))) {
    Debug("Failed to create new stream", 0);
    Free();
    return;
  }

  if (!(cctx = avcodec_alloc_context3(codec))) {
    Debug("Failed to allocate codec context", 0);
    Free();
    return;
  }

  videoStream->codecpar->codec_id = oformat->video_codec;
  videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
  videoStream->codecpar->width = width;
  videoStream->codecpar->height = height;
  videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
  videoStream->codecpar->bit_rate = bitrate * 1000;
  videoStream->time_base = {1, fps};

  avcodec_parameters_to_context(cctx, videoStream->codecpar);
  cctx->time_base = {1, fps};
  cctx->max_b_frames = 2;
  cctx->gop_size = 12;
  if (videoStream->codecpar->codec_id == AV_CODEC_ID_H264) {
    av_opt_set(cctx, "preset", "ultrafast", 0);
  }
  if (ofctx->oformat->flags & AVFMT_GLOBALHEADER) {
    cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  avcodec_parameters_from_context(videoStream->codecpar, cctx);

  if ((err = avcodec_open2(cctx, codec, NULL)) < 0) {
    Debug("Failed to open codec", err);
    Free();
    return;
  }

  if (!(oformat->flags & AVFMT_NOFILE)) {
    if ((err = avio_open(&ofctx->pb, VIDEO_TMP_FILE, AVIO_FLAG_WRITE)) < 0) {
      Debug("Failed to open file", err);
      Free();
      return;
    }
  }

  if ((err = avformat_write_header(ofctx, NULL)) < 0) {
    Debug("Failed to write header", err);
    Free();
    return;
  }

  av_dump_format(ofctx, 0, VIDEO_TMP_FILE, 1);
}

void VideoCapture::AddFrame(uint8_t *data) {
  int err;
  if (!videoFrame) {

    videoFrame = av_frame_alloc();
    videoFrame->format = AV_PIX_FMT_YUV420P;
    videoFrame->width = cctx->width;
    videoFrame->height = cctx->height;
    std::cout << "video channels " << videoFrame->channels << std::endl;
    if ((err = av_frame_get_buffer(videoFrame, 0)) < 0) {
      Debug("Failed to allocate picture", err);
      return;
    }
  }
  // AV_PIX_FMT_GRAY8
  if (!swsCtx) {
    swsCtx =
        sws_getContext(cctx->width, cctx->height, AV_PIX_FMT_GRAY8, cctx->width,
                       cctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
  }

  int inLinesize[1] = {1 * cctx->width};

  // From RGB to YUV
  sws_scale(swsCtx, (const uint8_t *const *)&data, inLinesize, 0, cctx->height,
            videoFrame->data, videoFrame->linesize);

  videoFrame->pts = frameCounter++;

  if ((err = avcodec_send_frame(cctx, videoFrame)) < 0) {
    Debug("Failed to send frame", err);
    return;
  }

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  if (avcodec_receive_packet(cctx, &pkt) == 0) {
    pkt.flags |= AV_PKT_FLAG_KEY;
    av_interleaved_write_frame(ofctx, &pkt);
    av_packet_unref(&pkt);
  }
}

void VideoCapture::Finish(const char *out_file_name) {
  // DELAYED FRAMES
  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;
  std::cout << "Finish" << std::endl;
  for (;;) {
    avcodec_send_frame(cctx, NULL);
    if (avcodec_receive_packet(cctx, &pkt) == 0) {
      av_interleaved_write_frame(ofctx, &pkt);
      av_packet_unref(&pkt);
    } else {
      break;
    }
  }

  av_write_trailer(ofctx);
  if (!(oformat->flags & AVFMT_NOFILE)) {
    int err = avio_close(ofctx->pb);
    if (err < 0) {
      Debug("Failed to close file", err);
    }
  }

  Free();

  Remux(out_file_name);
}

void VideoCapture::Free() {
  if (videoFrame) {
    av_frame_free(&videoFrame);
  }
  if (cctx) {
    avcodec_free_context(&cctx);
  }
  if (ofctx) {
    avformat_free_context(ofctx);
  }
  if (swsCtx) {
    sws_freeContext(swsCtx);
  }
}

void VideoCapture::Remux(const char *out_file_name) {
  AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
  int err;

  if ((err = avformat_open_input(&ifmt_ctx, VIDEO_TMP_FILE, 0, 0)) < 0) {
    Debug("Failed to open input file for remuxing", err);
    goto end;
  }
  if ((err = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
    Debug("Failed to retrieve input stream information", err);
    goto end;
  }
  if ((err = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL,
                                            out_file_name))) {
    Debug("Failed to allocate output context", err);
    goto end;
  }
  {
    AVStream *inVideoStream = ifmt_ctx->streams[0];
    AVStream *outVideoStream = avformat_new_stream(ofmt_ctx, NULL);
    if (!outVideoStream) {
      Debug("Failed to allocate output video stream", 0);
      goto end;
    }
    outVideoStream->time_base = {1, fps};
    avcodec_parameters_copy(outVideoStream->codecpar, inVideoStream->codecpar);
    outVideoStream->codecpar->codec_tag = 0;

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
      if ((err = avio_open(&ofmt_ctx->pb, out_file_name, AVIO_FLAG_WRITE)) <
          0) {
        Debug("Failed to open output file", err);
        goto end;
      }
    }

    if ((err = avformat_write_header(ofmt_ctx, 0)) < 0) {
      Debug("Failed to write header to output file", err);
      goto end;
    }

    AVPacket videoPkt;
    int ts = 0;
    while (true) {
      if ((err = av_read_frame(ifmt_ctx, &videoPkt)) < 0) {
        break;
      }
      videoPkt.stream_index = outVideoStream->index;
      videoPkt.pts = ts;
      videoPkt.dts = ts;
      videoPkt.duration =
          av_rescale_q(videoPkt.duration, inVideoStream->time_base,
                       outVideoStream->time_base);
      ts += videoPkt.duration;
      videoPkt.pos = -1;

      if ((err = av_interleaved_write_frame(ofmt_ctx, &videoPkt)) < 0) {
        Debug("Failed to mux packet", err);
        av_packet_unref(&videoPkt);
        break;
      }
      av_packet_unref(&videoPkt);
    }

    av_write_trailer(ofmt_ctx);
  }
end : {
  if (ifmt_ctx) {
    avformat_close_input(&ifmt_ctx);
  }
  if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    avio_closep(&ofmt_ctx->pb);
  }
  if (ofmt_ctx) {
    avformat_free_context(ofmt_ctx);
  }
}
}
