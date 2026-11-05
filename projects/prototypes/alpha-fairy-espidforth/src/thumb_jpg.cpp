/*
 * thumb_jpg.cpp — see thumb_jpg.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdlib.h>
#include <string.h>
#include <new>
#include "jpge.h"
#include "thumb_jpg.h"

namespace {
class buf_stream : public jpge::output_stream {
public:
    buf_stream(uint8_t *out, int cap) : m_out(out), m_cap(cap), m_len(0), m_full(false) {}
    virtual ~buf_stream() {}
    virtual bool put_buf(const void *data, int len) {
        if (m_len + len > m_cap) { m_full = true; return false; }
        memcpy(m_out + m_len, data, len);
        m_len += len;
        return true;
    }
    virtual unsigned int get_size() const { return (unsigned int)m_len; }
    uint8_t *m_out;
    int m_cap, m_len;
    bool m_full;
};
}

extern "C" int thumb_jpeg(const uint16_t *rgb565, int w, int h, int quality, uint8_t *out, int cap)
{
    buf_stream stream(out, cap);
    jpge::params p;
    p.m_quality = quality;
    p.m_subsampling = jpge::H2V2;
    jpge::jpeg_encoder *enc = new (std::nothrow) jpge::jpeg_encoder();
    uint8_t *line = (uint8_t *)malloc(w * 3);
    int n = 0;
    if (enc && line && enc->init(&stream, w, h, 3, p)) {
        bool ok = true;
        for (int y = 0; y < h && ok; y++) {
            const uint16_t *s = rgb565 + y * w;
            for (int x = 0; x < w; x++) {
                uint16_t v = s[x];
                line[3 * x]     = (uint8_t)((v >> 8) & 0xF8);
                line[3 * x + 1] = (uint8_t)((v >> 3) & 0xFC);
                line[3 * x + 2] = (uint8_t)((v << 3) & 0xF8);
            }
            ok = enc->process_scanline(line);
        }
        if (ok && enc->process_scanline(NULL) && !stream.m_full) n = stream.m_len;
        enc->deinit();
    }
    delete enc;
    free(line);
    return n;
}

namespace {
struct stream_enc {
    buf_stream stream;
    jpge::jpeg_encoder enc;
    bool ok;
    stream_enc(uint8_t *out, int cap) : stream(out, cap), ok(true) {}
};
}

extern "C" void *thumb_stream_begin(int w, int h, int quality, uint8_t *out, int cap)
{
    stream_enc *s = new (std::nothrow) stream_enc(out, cap);
    if (!s) return NULL;
    jpge::params p;
    p.m_quality = quality;
    p.m_subsampling = jpge::H2V2;
    if (!s->enc.init(&s->stream, w, h, 3, p)) { delete s; return NULL; }
    return s;
}

extern "C" bool thumb_stream_line(void *enc, const uint8_t *rgb888)
{
    stream_enc *s = (stream_enc *)enc;
    if (s->ok) s->ok = s->enc.process_scanline(rgb888);
    return s->ok;
}

extern "C" int thumb_stream_end(void *enc)
{
    stream_enc *s = (stream_enc *)enc;
    int n = 0;
    if (s->ok && s->enc.process_scanline(NULL) && !s->stream.m_full) n = s->stream.m_len;
    s->enc.deinit();
    delete s;
    return n;
}
