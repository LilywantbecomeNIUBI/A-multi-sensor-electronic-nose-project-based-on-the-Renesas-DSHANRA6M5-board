/*
 * face_simple.h
 *
 *  Created on: 2026年3月25日
 *      Author: leo
 */

#ifndef FACE_SIMPLE_H
#define FACE_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"
#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>


typedef enum e_face_simple_emotion
{
    FACE_SIMPLE_HAPPY = 0,
    FACE_SIMPLE_SAD,
    FACE_SIMPLE_ALERT,
    FACE_SIMPLE_THINKING,
    FACE_SIMPLE_AIR,
    FACE_SIMPLE_EMOTION_MAX
} face_simple_emotion_t;

typedef struct st_face_simple_cfg
{
    lv_obj_t * p_parent;
    lv_coord_t width;
    lv_coord_t height;
    lv_coord_t pos_x;
    lv_coord_t pos_y;
    bool auto_blink;
    uint32_t blink_period_ms;
} face_simple_cfg_t;

fsp_err_t face_simple_init(face_simple_cfg_t const * p_cfg);
void face_simple_deinit(void);

void face_simple_set_emotion(face_simple_emotion_t emotion);
face_simple_emotion_t face_simple_get_emotion(void);

void face_simple_set_position(lv_coord_t x, lv_coord_t y);
lv_obj_t * face_simple_get_container(void);

void face_simple_trigger_blink(void);

#ifdef __cplusplus
}
#endif

#endif /* FACE_SIMPLE_H */
