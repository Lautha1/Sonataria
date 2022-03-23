#pragma once

#include <stdio.h>
#include <stdint.h>

#include <GL/glew.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class AVDecode
{
public:
	AVDecode();
	~AVDecode();

	bool prepareToDecode(const char* src_filename);
	double decodeFrame();
	void closeDecoder();

	void setTextureIDs(GLuint yTexID = 0, GLuint uTexID = 0, GLuint vTexID = 0);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

protected:
	int width, height;
	FILE* video_dst_file;

	AVFormatContext* fmt_ctx;
	AVCodecContext* video_dec_ctx;
	enum AVPixelFormat pix_fmt;
	AVStream* video_stream;

	uint8_t* video_dst_data[4];
	int video_dst_linesize[4];
	int video_dst_bufsize;

	int video_stream_idx;
	AVFrame* frame;
	AVPacket* pkt;
	int video_frame_count;
	double pkt_time_base;
	double curFrameTimestampMsec;

	GLuint textureIDs[3];

	bool theEnd(bool result);
	static int open_codec_context(const char* src_filename, int* stream_idx,
		AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, enum AVMediaType type);

	int decode_packet(AVCodecContext* dec, const AVPacket* pkt);
	void send_frame_to_GPU(AVFrame* frame);
	int output_video_frame(AVFrame* frame);
};
