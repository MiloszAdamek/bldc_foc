/*
 * encoder_types.h
 *
 *  Created on: 3 Oct 2026
 *      Author: Milosz Adamek
 */

 #ifndef INC_FOC_ENCODER_TYPES_H_
 #define INC_FOC_ENCODER_TYPES_H_

 #include <stdint.h>
 #include <stdbool.h>

typedef enum
{
    ENCODER_AS5048A_ABSOLUTE,
    ENCODER_INCREMENTAL
} EncoderType_t;

typedef struct
{
    EncoderType_t type;

    uint32_t cpr;
    uint8_t pole_pairs;

    bool has_index_z;

    int8_t direction;
    int32_t zero_count;
    float zero_electric_angle;

} EncoderConfig_t;

typedef struct
{
    float theta_raw;
    float theta_predicted;
    float dt;
    bool new_sample;
    bool valid;
} EncoderAngle_t;

/* Surowa próbka z enkodera - wypełniana TYLKO przez SPI DMA callback */
typedef struct {
    uint16_t  raw;          /* 14-bit pozycja */
    float     theta_mech;   /* [rad] 0..2pi */
    uint32_t  tick;         /* DWT timestamp przy odebraniu */
    bool      valid;
} EncoderSample_t;

/* Snapshot kąta publikowany przez pętlę FOC */
typedef struct {
    volatile uint32_t seq;
    float     theta_mech;   /* [rad] mechaniczny */
    float     theta_el;     /* [rad] elektryczny  */
    uint32_t  tick;
} AngleSnapshot_t;

typedef struct
{
    bool (*init)(void);
    bool (*update)(EncoderSample_t *sample);
    void (*set_zero)(void);
} EncoderDriver_t;

#endif /* INC_FOC_ENCODER_TYPES_H_ */