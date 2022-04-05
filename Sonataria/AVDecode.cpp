/**
 * This code is largely based on the demuxing and decoding example from the
 * docs/examples folder of the ffmpeg source code (demuxing_decoding.c).
 */

#include "AVDecode.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
}

AVDecode::AVDecode()
{
    // Zero memory in member buffers
    memset(video_dst_data, 0, sizeof(video_dst_data));
    memset(video_dst_linesize, 0, sizeof(video_dst_linesize));

    // Set defaults for all member variables
    fmt_ctx = NULL;
    video_dec_ctx = NULL;
    width = height = 0;
    pix_fmt = AV_PIX_FMT_NONE;
    video_stream = NULL;
    video_dst_file = NULL;
    video_dst_bufsize = 0;
    video_stream_idx = -1;
    frame = NULL;
    pkt = NULL;
    video_frame_count = 0;

    // Initialize texture IDs to undefined
    setTextureIDs();
}

AVDecode::~AVDecode() { theEnd(true); }

void AVDecode::setTextureIDs(GLuint yTexID, GLuint uTexID, GLuint vTexID)
{
    // Update OpenGL texture IDs for passing data to GPU
    textureIDs[0] = yTexID;
    textureIDs[1] = uTexID;
    textureIDs[2] = vTexID;
}

int AVDecode::output_video_frame(AVFrame* frame)
{
    if (frame->width != width || frame->height != height ||
        frame->format != pix_fmt) {
        /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
        fprintf(stderr, "Error: Width, height and pixel format have to be "
            "constant in a rawvideo file, but the width, height or "
            "pixel format of the input video changed:\n"
            "old: width = %d, height = %d, format = %s\n"
            "new: width = %d, height = %d, format = %s\n",
            width, height, av_get_pix_fmt_name(pix_fmt),
            frame->width, frame->height,
            av_get_pix_fmt_name((AVPixelFormat)frame->format));
        return -1;
    }

    /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
    av_image_copy(video_dst_data, video_dst_linesize,
        (const uint8_t**)(frame->data), frame->linesize,
        pix_fmt, width, height);

    /* write to rawvideo file */
    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
    return 0;
}

void AVDecode::send_frame_to_GPU(AVFrame* frame)
{
    if (textureIDs[0] != 0 && frame->data[0])
    {
        // Pass in the Y luminance plane as a GL_RED texture (e.g. one color channel)
        glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[0]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height,
            GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);
    }

    if (textureIDs[1] != 0 && frame->data[1])
    {
        // Pass in the u color plane as a GL_RED texture (also one color channel)
        glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[1]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width / 2, frame->height / 2,
            GL_RED, GL_UNSIGNED_BYTE, frame->data[1]);
    }

    if (textureIDs[2] != 0 && frame->data[2])
    {
        // Pass in the v color plane as a GL_RED texture (also one color channel)
        glBindTexture(GL_TEXTURE_2D, textureIDs[2]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[2]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width / 2, frame->height / 2,
            GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);
    }
}

int AVDecode::decode_packet(AVCodecContext* dec, const AVPacket* pkt)
{
    // submit the packet to the decoder
    int ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding\n");
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding\n");
            return ret;
        }

        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
            send_frame_to_GPU(frame);
            // ret = output_video_frame(frame);
        }

        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }

    return 0;
}

int AVDecode::open_codec_context(const char* src_filename, int* stream_idx,
    AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream* st;
    const AVCodec* dec = NULL;
    AVDictionary* opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
            av_get_media_type_string(type), src_filename);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

inline bool AVDecode::theEnd(bool result) {
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    if (video_dst_file) {
        fclose(video_dst_file);
        video_dst_file = NULL;
    }
    av_packet_free(&pkt);
    av_frame_free(&frame);
    if (video_dst_data[0]) {
        av_free(video_dst_data[0]);
        memset(video_dst_data, 0, sizeof(video_dst_data));
    }

    fmt_ctx = NULL;
    video_dec_ctx = NULL;
    pkt = NULL;
    frame = NULL;

    return result;
}

bool AVDecode::prepareToDecode(const char* src_filename) {
    int ret;

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        return false;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return false;
    }

    if (open_codec_context(src_filename, &video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];

        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
            width, height, pix_fmt, 1);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            return theEnd(false);
        }
        video_dst_bufsize = ret;
    }

    /* Compute time base (in seconds) for pkt->pts */
    pkt_time_base = video_dec_ctx->pkt_timebase.num/(double)video_dec_ctx->pkt_timebase.den;

    /* dump input information to stderr */
    //av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        return theEnd(false);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        return theEnd(false);
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate packet\n");
        return theEnd(false);
    }

    return true;
}

double AVDecode::decodeFrame() {
    /* read frames from the file */
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_idx) {
            curFrameTimestampMsec = pkt->pts * pkt_time_base;
            int ret = decode_packet(video_dec_ctx, pkt);
            av_packet_unref(pkt);
            if (ret < 0) { return -1.0; }
            return curFrameTimestampMsec;
        }

        // Not a video packet so get another one
        av_packet_unref(pkt);
    }

    return -1.0;
}

void AVDecode::closeDecoder() {
    /* flush the decoder */
    if (video_dec_ctx) {
        decode_packet(video_dec_ctx, NULL);
    }

    theEnd(true);
}
