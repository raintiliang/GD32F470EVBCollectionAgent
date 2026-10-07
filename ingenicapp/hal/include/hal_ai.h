// HAL AI Abstraction
// Unified interface for AI inference and computer vision

#ifndef _HAL_AI_H_
#define _HAL_AI_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI model configuration
typedef struct {
    hal_ai_model_type_t type;        // Model type
    char model_path[256];            // Path to model file
    char param_path[256];            // Path to parameter file (optional)
    uint32_t input_width;            // Model input width
    uint32_t input_height;           // Model input height
    uint32_t input_channels;         // Model input channels (1, 3, 4)
    hal_pixel_format_t input_format; // Input pixel format
    uint32_t max_batch_size;         // Maximum batch size
    bool use_gpu;                    // Use GPU acceleration
    bool use_npu;                    // Use NPU acceleration
    uint32_t priority;               // Inference priority (0-100)
    void *priv;                      // Platform private config
} hal_ai_model_config_t;

// AI inference configuration
typedef struct {
    uint32_t model_id;               // Model ID returned by hal_ai_load_model
    uint32_t batch_size;             // Batch size for inference
    float confidence_threshold;      // Detection confidence threshold (0-1)
    float nms_threshold;             // Non-maximum suppression threshold (0-1)
    uint32_t max_detections;         // Maximum number of detections per frame
    bool enable_preprocess;          // Enable preprocessing
    bool enable_postprocess;         // Enable postprocessing
    void *preprocess_params;         // Preprocessing parameters
    void *postprocess_params;        // Postprocessing parameters
} hal_ai_inference_config_t;

// AI preprocessing operations
typedef enum {
    HAL_AI_PREPROCESS_RESIZE = 0,    // Resize to model input size
    HAL_AI_PREPROCESS_CROP,          // Center crop
    HAL_AI_PREPROCESS_NORMALIZE,     // Normalize pixel values
    HAL_AI_PREPROCESS_MEAN_SUBTRACT, // Subtract mean value
    HAL_AI_PREPROCESS_SCALE,         // Scale pixel values
    HAL_AI_PREPROCESS_COLOR_CONVERT, // Color space conversion
} hal_ai_preprocess_op_t;

// AI postprocessing operations
typedef enum {
    HAL_AI_POSTPROCESS_DETECTION = 0, // Object detection
    HAL_AI_POSTPROCESS_CLASSIFICATION, // Image classification
    HAL_AI_POSTPROCESS_SEGMENTATION,   // Semantic segmentation
    HAL_AI_POSTPROCESS_POSE_ESTIMATION, // Pose estimation
    HAL_AI_POSTPROCESS_FACE_RECOGNITION, // Face recognition
} hal_ai_postprocess_op_t;

// AI model handle
typedef void* hal_ai_model_handle_t;

// AI inference session handle
typedef void* hal_ai_session_handle_t;

// Model management
int hal_ai_init(void);
int hal_ai_deinit(void);
int hal_ai_load_model(const hal_ai_model_config_t *config, hal_ai_model_handle_t *model);
int hal_ai_unload_model(hal_ai_model_handle_t model);
int hal_ai_get_model_info(hal_ai_model_handle_t model, hal_ai_model_config_t *info);

// Inference session management
int hal_ai_create_session(hal_ai_model_handle_t model, 
                          const hal_ai_inference_config_t *config, 
                          hal_ai_session_handle_t *session);
int hal_ai_destroy_session(hal_ai_session_handle_t session);
int hal_ai_start_session(hal_ai_session_handle_t session);
int hal_ai_stop_session(hal_ai_session_handle_t session);

// Single frame inference
int hal_ai_inference_frame(hal_ai_session_handle_t session, 
                          const hal_frame_t *input_frame, 
                          hal_ai_result_t *result);

// Batch inference
int hal_ai_inference_batch(hal_ai_session_handle_t session, 
                          const hal_frame_t *input_frames, 
                          uint32_t frame_count, 
                          hal_ai_result_t *results);

// Async inference
typedef void (*hal_ai_callback_t)(hal_ai_session_handle_t session, 
                                  const hal_frame_t *input_frame, 
                                  const hal_ai_result_t *result, 
                                  void *user_data);

int hal_ai_inference_async(hal_ai_session_handle_t session, 
                          const hal_frame_t *input_frame, 
                          hal_ai_callback_t callback, 
                          void *user_data);
int hal_ai_wait_inference(hal_ai_session_handle_t session, int timeout_ms);

// Common AI tasks
int hal_ai_face_detect(const hal_frame_t *frame, hal_ai_result_t *result);
int hal_ai_face_recognize(const hal_frame_t *frame, const hal_frame_t *face, 
                         float *similarity, uint32_t *person_id);
int hal_ai_person_detect(const hal_frame_t *frame, hal_ai_result_t *result);
int hal_ai_vehicle_detect(const hal_frame_t *frame, hal_ai_result_t *result);
int hal_ai_license_plate_recognize(const hal_frame_t *frame, char *plate_text, size_t max_len);
int hal_ai_motion_detect(const hal_frame_t *frame, bool *motion_detected, hal_rect_t *motion_area);
int hal_ai_face_quality_assess(const hal_frame_t *face_frame, float *quality_score);

// AI preprocessing
int hal_ai_preprocess_frame(const hal_frame_t *input_frame, 
                           hal_frame_t *output_frame, 
                           const hal_ai_preprocess_op_t *ops, 
                           uint32_t op_count, 
                           void *params);

int hal_ai_resize_frame(const hal_frame_t *input_frame, 
                       uint32_t target_width, 
                       uint32_t target_height, 
                       hal_frame_t *output_frame);

int hal_ai_crop_frame(const hal_frame_t *input_frame, 
                     const hal_rect_t *crop_rect, 
                     hal_frame_t *output_frame);

int hal_ai_normalize_frame(const hal_frame_t *input_frame, 
                          float mean_r, float mean_g, float mean_b,
                          float std_r, float std_g, float std_b,
                          hal_frame_t *output_frame);

int hal_ai_convert_color(const hal_frame_t *input_frame, 
                        hal_pixel_format_t target_format, 
                        hal_frame_t *output_frame);

// AI postprocessing
int hal_ai_postprocess_detection(const void *model_output, 
                                uint32_t output_size, 
                                const hal_ai_inference_config_t *config, 
                                hal_ai_result_t *result);

int hal_ai_postprocess_classification(const void *model_output, 
                                     uint32_t output_size, 
                                     uint32_t *class_id, 
                                     float *confidence);

int hal_ai_postprocess_segmentation(const void *model_output, 
                                   uint32_t output_size, 
                                   uint32_t width, 
                                   uint32_t height, 
                                   uint8_t *segmentation_mask);

// Feature extraction
int hal_ai_extract_feature(const hal_frame_t *frame, 
                          float *feature_vector, 
                          uint32_t *feature_dim);

int hal_ai_compare_features(const float *feature1, 
                           const float *feature2, 
                           uint32_t feature_dim, 
                           float *similarity);

// AI hardware acceleration
typedef struct {
    bool npu_available;
    bool gpu_available;
    bool dsp_available;
    uint32_t npu_freq;      // NPU frequency (MHz)
    uint32_t npu_cores;     // Number of NPU cores
    uint32_t gpu_memory;    // GPU memory (MB)
    uint32_t max_power;     // Maximum power consumption (mW)
} hal_ai_hw_info_t;

int hal_ai_get_hardware_info(hal_ai_hw_info_t *info);
int hal_ai_set_power_mode(uint32_t mode);  // 0: low power, 1: balanced, 2: high performance
int hal_ai_set_priority(uint32_t priority); // 0-100

// AI memory management
int hal_ai_alloc_buffer(uint32_t size, hal_buffer_t *buffer);
int hal_ai_free_buffer(const hal_buffer_t *buffer);
int hal_ai_cache_model(hal_ai_model_handle_t model);
int hal_ai_clear_cache(void);

// AI statistics
typedef struct {
    uint32_t inference_count;
    uint32_t success_count;
    uint32_t error_count;
    uint32_t total_inference_time;   // Total time in microseconds
    uint32_t avg_inference_time;     // Average time in microseconds
    uint32_t max_inference_time;     // Maximum time in microseconds
    uint32_t min_inference_time;     // Minimum time in microseconds
    uint32_t fps;                    // Current inference FPS
    uint32_t memory_usage;           // Memory usage in KB
} hal_ai_stats_t;

int hal_ai_get_stats(hal_ai_session_handle_t session, hal_ai_stats_t *stats);
int hal_ai_reset_stats(hal_ai_session_handle_t session);

// Platform-specific AI operations
int hal_ai_platform_init(void);
int hal_ai_platform_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // _HAL_AI_H_