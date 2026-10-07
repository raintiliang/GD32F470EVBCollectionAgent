// HAL Video Abstraction
// Unified interface for video capture, processing and encoding

#ifndef _HAL_VIDEO_H_
#define _HAL_VIDEO_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Video input configuration
typedef struct {
    uint32_t dev_id;                   // Device ID
    uint32_t width;                    // Input width
    uint32_t height;                   // Input height
    hal_pixel_format_t pixel_format;   // Pixel format
    uint32_t fps;                      // Frame rate
    uint32_t mipi_lane;                // MIPI lane count
    uint32_t mipi_freq;                // MIPI frequency (MHz)
    bool hdr_enable;                   // Enable HDR
    bool wdr_enable;                   // Enable WDR
    uint32_t sensor_id;                // Sensor ID
    void *sensor_priv;                 // Sensor private data
} hal_vi_config_t;

// Video processing configuration
typedef struct {
    uint32_t chn_id;                   // VPSS channel ID
    hal_rect_t crop_rect;              // Crop rectangle
    hal_size_t output_size;            // Output size
    hal_pixel_format_t output_format;  // Output pixel format
    bool mirror_enable;                // Horizontal mirror
    bool flip_enable;                  // Vertical flip
    uint32_t rotation;                 // Rotation angle (0, 90, 180, 270)
    uint32_t nr_level;                 // Noise reduction level (0-100)
    uint32_t sharpness_level;          // Sharpness level (0-100)
    uint32_t brightness;               // Brightness (-100 to 100)
    uint32_t contrast;                 // Contrast (-100 to 100)
    uint32_t saturation;               // Saturation (-100 to 100)
    uint32_t hue;                      // Hue (-180 to 180)
} hal_vpss_config_t;

// Video encoder configuration
typedef struct {
    uint32_t chn_id;                   // Encoder channel ID
    hal_video_codec_t codec;           // Codec type
    uint32_t width;                    // Encode width
    uint32_t height;                   // Encode height
    uint32_t fps;                      // Frame rate
    uint32_t gop;                      // GOP size
    uint32_t bitrate;                  // Bitrate (bps)
    uint32_t max_bitrate;              // Max bitrate (bps)
    uint32_t profile;                  // H.264: 66(baseline), 77(main), 100(high)
    uint32_t level;                    // Codec level
    uint32_t rc_mode;                  // Rate control mode: 0-CBR, 1-VBR, 2-FIXQP
    uint32_t i_qp;                     // I frame QP
    uint32_t p_qp;                     // P frame QP
    uint32_t b_qp;                     // B frame QP (if supported)
    bool smart_encode;                 // Enable smart encoding
    bool slice_encode;                 // Enable slice encoding
    void *priv;                        // Platform private config
} hal_venc_config_t;

// ISP configuration
typedef struct {
    bool ae_enable;                    // Auto exposure enable
    bool awb_enable;                   // Auto white balance enable
    bool af_enable;                    // Auto focus enable
    uint32_t exposure_time;            // Exposure time (us)
    uint32_t analog_gain;              // Analog gain (0-100)
    uint32_t digital_gain;             // Digital gain (0-100)
    uint32_t isp_gain;                 // ISP gain (0-100)
    uint32_t wb_mode;                  // White balance mode
    uint32_t wb_r_gain;                // Red gain for WB
    uint32_t wb_g_gain;                // Green gain for WB
    uint32_t wb_b_gain;                // Blue gain for WB
    uint32_t black_level;              // Black level
    uint32_t gamma_curve;              // Gamma curve index
    uint32_t defect_pixel_correct;     // Defect pixel correction
    uint32_t lens_shading_correct;     // Lens shading correction
    void *tuning_data;                 // ISP tuning data
    size_t tuning_size;                // ISP tuning data size
} hal_isp_config_t;

// Video input operations
int hal_vi_init(void);
int hal_vi_deinit(void);
int hal_vi_create_device(const hal_vi_config_t *config, uint32_t *dev_id);
int hal_vi_destroy_device(uint32_t dev_id);
int hal_vi_start_capture(uint32_t dev_id);
int hal_vi_stop_capture(uint32_t dev_id);
int hal_vi_get_frame(uint32_t dev_id, hal_frame_t *frame, int timeout_ms);
int hal_vi_release_frame(uint32_t dev_id, const hal_frame_t *frame);

// Video processing operations
int hal_vpss_init(void);
int hal_vpss_deinit(void);
int hal_vpss_create_channel(const hal_vpss_config_t *config, uint32_t *chn_id);
int hal_vpss_destroy_channel(uint32_t chn_id);
int hal_vpss_start(uint32_t chn_id);
int hal_vpss_stop(uint32_t chn_id);
int hal_vpss_process_frame(uint32_t chn_id, const hal_frame_t *in_frame, hal_frame_t *out_frame);
int hal_vpss_bind_vi(uint32_t vpss_chn_id, uint32_t vi_dev_id, uint32_t vi_chn_id);
int hal_vpss_unbind_vi(uint32_t vpss_chn_id);

// Video encoder operations
int hal_venc_init(void);
int hal_venc_deinit(void);
int hal_venc_create_channel(const hal_venc_config_t *config, uint32_t *chn_id);
int hal_venc_destroy_channel(uint32_t chn_id);
int hal_venc_start(uint32_t chn_id);
int hal_venc_stop(uint32_t chn_id);
int hal_venc_encode_frame(uint32_t chn_id, const hal_frame_t *frame, hal_frame_t *encoded_frame);
int hal_venc_request_idr(uint32_t chn_id);
int hal_venc_set_bitrate(uint32_t chn_id, uint32_t bitrate);
int hal_venc_set_framerate(uint32_t chn_id, uint32_t fps);
int hal_venc_get_stream(uint32_t chn_id, hal_frame_t *stream_frame, int timeout_ms);
int hal_venc_release_stream(uint32_t chn_id, const hal_frame_t *stream_frame);
int hal_venc_bind_vpss(uint32_t venc_chn_id, uint32_t vpss_chn_id);
int hal_venc_unbind_vpss(uint32_t venc_chn_id);

// ISP operations
int hal_isp_init(void);
int hal_isp_deinit(void);
int hal_isp_load_tuning_data(const void *data, size_t size);
int hal_isp_set_config(const hal_isp_config_t *config);
int hal_isp_get_config(hal_isp_config_t *config);
int hal_isp_ae_enable(bool enable);
int hal_isp_awb_enable(bool enable);
int hal_isp_af_enable(bool enable);
int hal_isp_set_exposure(uint32_t exposure_time);
int hal_isp_set_gain(uint32_t analog_gain, uint32_t digital_gain);
int hal_isp_set_white_balance(uint32_t mode, uint32_t r_gain, uint32_t g_gain, uint32_t b_gain);
int hal_isp_set_brightness(uint32_t value);
int hal_isp_set_contrast(uint32_t value);
int hal_isp_set_saturation(uint32_t value);
int hal_isp_set_sharpness(uint32_t value);
int hal_isp_set_hue(uint32_t value);
int hal_isp_set_nr_level(uint32_t level);
int hal_isp_set_defect_pixel_correct(bool enable);
int hal_isp_set_lens_shading_correct(bool enable);

// Sensor control
typedef struct {
    char name[32];
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint32_t fps_max;
    uint32_t pixel_format;
    bool hdr_support;
    bool wdr_support;
} hal_sensor_info_t;

int hal_sensor_probe(hal_sensor_info_t *info, uint32_t max_count, uint32_t *count);
int hal_sensor_init(uint32_t sensor_id);
int hal_sensor_deinit(uint32_t sensor_id);
int hal_sensor_reset(uint32_t sensor_id);
int hal_sensor_standby(uint32_t sensor_id);
int hal_sensor_wakeup(uint32_t sensor_id);

// Video statistics
typedef struct {
    uint32_t frame_count;
    uint32_t frame_rate;
    uint32_t bitrate;
    uint32_t encode_time;      // Average encode time (us)
    uint32_t drop_frame_count;
    uint32_t error_frame_count;
} hal_video_stats_t;

int hal_video_get_stats(uint32_t chn_id, hal_video_stats_t *stats);
int hal_video_reset_stats(uint32_t chn_id);

#ifdef __cplusplus
}
#endif

#endif // _HAL_VIDEO_H_