// HAL Audio Abstraction
// Unified interface for audio capture and playback

#ifndef _HAL_AUDIO_H_
#define _HAL_AUDIO_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Audio sample format
typedef enum {
    HAL_AUDIO_FORMAT_PCM_S16_LE = 0,  // Signed 16-bit little-endian
    HAL_AUDIO_FORMAT_PCM_S16_BE,      // Signed 16-bit big-endian
    HAL_AUDIO_FORMAT_PCM_U16_LE,      // Unsigned 16-bit little-endian
    HAL_AUDIO_FORMAT_PCM_U16_BE,      // Unsigned 16-bit big-endian
    HAL_AUDIO_FORMAT_PCM_S32_LE,      // Signed 32-bit little-endian
    HAL_AUDIO_FORMAT_PCM_S32_BE,      // Signed 32-bit big-endian
    HAL_AUDIO_FORMAT_PCM_F32_LE,      // Float 32-bit little-endian
    HAL_AUDIO_FORMAT_PCM_F32_BE,      // Float 32-bit big-endian
} hal_audio_format_t;

// Audio channel configuration
typedef enum {
    HAL_AUDIO_CHANNEL_MONO = 1,
    HAL_AUDIO_CHANNEL_STEREO = 2,
} hal_audio_channel_t;

// Audio device type
typedef enum {
    HAL_AUDIO_DEVICE_MIC = 0,      // Microphone input
    HAL_AUDIO_DEVICE_LINE_IN,      // Line input
    HAL_AUDIO_DEVICE_SPEAKER,      // Speaker output
    HAL_AUDIO_DEVICE_LINE_OUT,     // Line output
    HAL_AUDIO_DEVICE_HEADPHONE,    // Headphone output
} hal_audio_device_t;

// Audio configuration
typedef struct {
    hal_audio_device_t device_type;   // Device type
    uint32_t sample_rate;            // Sample rate (Hz): 8000, 16000, 32000, 44100, 48000
    hal_audio_format_t format;       // Sample format
    hal_audio_channel_t channels;    // Channel count
    uint32_t period_size;            // Period size in frames
    uint32_t period_count;           // Number of periods
    uint32_t volume;                 // Volume level (0-100)
    bool loopback_enable;            // Enable loopback
    bool aec_enable;                 // Enable acoustic echo cancellation
    bool ns_enable;                  // Enable noise suppression
    bool agc_enable;                 // Enable automatic gain control
    void *priv;                      // Platform private config
} hal_audio_config_t;

// Audio frame
typedef struct {
    hal_buffer_t buf;                // Audio buffer
    uint64_t pts;                    // Presentation timestamp (us)
    uint32_t seq;                    // Frame sequence number
    uint32_t sample_rate;            // Sample rate
    hal_audio_format_t format;       // Sample format
    hal_audio_channel_t channels;    // Channel count
    uint32_t samples;                // Number of samples per channel
    uint32_t size;                   // Actual data size
    void *priv;                      // Platform private data
} hal_audio_frame_t;

// Audio encoder configuration
typedef struct {
    hal_audio_codec_t codec;         // Audio codec
    uint32_t sample_rate;            // Input sample rate
    hal_audio_channel_t channels;    // Input channels
    uint32_t bitrate;                // Bitrate (bps)
    uint32_t complexity;             // Encoding complexity (0-10)
    uint32_t profile;                // Codec profile
    void *priv;                      // Platform private config
} hal_aenc_config_t;

// Audio decoder configuration
typedef struct {
    hal_audio_codec_t codec;         // Audio codec
    uint32_t sample_rate;            // Output sample rate
    hal_audio_channel_t channels;    // Output channels
    void *priv;                      // Platform private config
} hal_adec_config_t;

// Audio device operations
int hal_audio_init(void);
int hal_audio_deinit(void);
int hal_audio_create_device(const hal_audio_config_t *config, uint32_t *dev_id);
int hal_audio_destroy_device(uint32_t dev_id);
int hal_audio_start(uint32_t dev_id);
int hal_audio_stop(uint32_t dev_id);
int hal_audio_pause(uint32_t dev_id);
int hal_audio_resume(uint32_t dev_id);

// Audio capture operations
int hal_audio_capture_frame(uint32_t dev_id, hal_audio_frame_t *frame, int timeout_ms);
int hal_audio_release_frame(uint32_t dev_id, const hal_audio_frame_t *frame);

// Audio playback operations
int hal_audio_play_frame(uint32_t dev_id, const hal_audio_frame_t *frame);
int hal_audio_drain(uint32_t dev_id);  // Wait for all queued frames to play

// Volume control
int hal_audio_set_volume(uint32_t dev_id, uint32_t volume);
int hal_audio_get_volume(uint32_t dev_id, uint32_t *volume);
int hal_audio_set_mute(uint32_t dev_id, bool mute);
int hal_audio_get_mute(uint32_t dev_id, bool *mute);

// Audio encoder operations
int hal_aenc_init(void);
int hal_aenc_deinit(void);
int hal_aenc_create_channel(const hal_aenc_config_t *config, uint32_t *chn_id);
int hal_aenc_destroy_channel(uint32_t chn_id);
int hal_aenc_start(uint32_t chn_id);
int hal_aenc_stop(uint32_t chn_id);
int hal_aenc_encode_frame(uint32_t chn_id, const hal_audio_frame_t *pcm_frame, hal_audio_frame_t *encoded_frame);
int hal_aenc_get_stream(uint32_t chn_id, hal_audio_frame_t *stream_frame, int timeout_ms);
int hal_aenc_release_stream(uint32_t chn_id, const hal_audio_frame_t *stream_frame);

// Audio decoder operations
int hal_adec_init(void);
int hal_adec_deinit(void);
int hal_adec_create_channel(const hal_adec_config_t *config, uint32_t *chn_id);
int hal_adec_destroy_channel(uint32_t chn_id);
int hal_adec_start(uint32_t chn_id);
int hal_adec_stop(uint32_t chn_id);
int hal_adec_decode_frame(uint32_t chn_id, const hal_audio_frame_t *encoded_frame, hal_audio_frame_t *pcm_frame);

// Audio processing
int hal_audio_enable_aec(uint32_t dev_id, bool enable);
int hal_audio_enable_ns(uint32_t dev_id, bool enable);
int hal_audio_enable_agc(uint32_t dev_id, bool enable);
int hal_audio_set_aec_params(uint32_t dev_id, int delay_ms, int filter_length);
int hal_audio_set_ns_params(uint32_t dev_id, int level);  // 0-3: off, low, moderate, high
int hal_audio_set_agc_params(uint32_t dev_id, int target_level_db, int compression_gain_db);

// Audio routing
int hal_audio_route_capture_to_encoder(uint32_t capture_dev_id, uint32_t aenc_chn_id);
int hal_audio_route_decoder_to_playback(uint32_t adec_chn_id, uint32_t playback_dev_id);
int hal_audio_route_capture_to_playback(uint32_t capture_dev_id, uint32_t playback_dev_id);  // Loopback

// Audio status
typedef struct {
    uint32_t frame_count;
    uint32_t sample_rate;
    uint32_t bitrate;
    uint32_t volume;
    bool mute;
    bool active;
} hal_audio_status_t;

int hal_audio_get_status(uint32_t dev_id, hal_audio_status_t *status);
int hal_audio_reset_status(uint32_t dev_id);

#ifdef __cplusplus
}
#endif

#endif // _HAL_AUDIO_H_