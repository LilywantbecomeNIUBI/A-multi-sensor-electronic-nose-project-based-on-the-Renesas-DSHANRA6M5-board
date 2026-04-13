/*
 * face_simple.c  —— 最终整合版 (联想云朵 + 大问号 + 底部状态字 + 吹口哨空气表情)
 */

#include "face_simple.h"
#include <string.h>

extern void uart_printf(const char * format, ...);

typedef struct st_face_simple_ctx
{
    bool inited;
    bool auto_blink;
    bool is_blinking;

    uint32_t blink_period_ms;
    uint32_t normal_blink_period_ms;

    lv_coord_t width;
    lv_coord_t height;

    face_simple_emotion_t emotion;

    lv_obj_t * p_parent;
    lv_obj_t * p_container;
    lv_obj_t * p_left_eye;
    lv_obj_t * p_right_eye;
    lv_obj_t * p_left_brow;
    lv_obj_t * p_right_brow;
    lv_obj_t * p_mouth_line;
    lv_obj_t * p_mouth_o;
    lv_obj_t * p_tear;

    /* 新增：thinking 联想云容器 */
    lv_obj_t * p_cloud_container;
    /* thinking 问号标签 */
    lv_obj_t * p_question_label;

    /* 底部状态文字标签 */
    lv_obj_t * p_status_label;

    lv_timer_t * p_blink_timer;
    lv_timer_t * p_thinking_anim_timer;

    uint8_t thinking_mouth_phase;
    uint8_t thinking_brow_phase;
    uint8_t thinking_anim_tick;

    lv_point_t left_brow_pts[2];
    lv_point_t right_brow_pts[2];
    lv_point_t mouth_pts[3];
} face_simple_ctx_t;

static face_simple_ctx_t g_face_simple = {0};

static lv_coord_t face_pct(lv_coord_t total, uint8_t pct)
{
    return (lv_coord_t) ((total * pct) / 100);
}

static lv_coord_t face_min2(lv_coord_t a, lv_coord_t b)
{
    return (a < b) ? a : b;
}

static lv_coord_t face_min3(lv_coord_t a, lv_coord_t b, lv_coord_t c)
{
    lv_coord_t min_ab = (a < b) ? a : b;
    return (min_ab < c) ? min_ab : c;
}

static lv_coord_t face_max2(lv_coord_t a, lv_coord_t b)
{
    return (a > b) ? a : b;
}

static lv_coord_t face_max3(lv_coord_t a, lv_coord_t b, lv_coord_t c)
{
    lv_coord_t max_ab = (a > b) ? a : b;
    return (max_ab > c) ? max_ab : c;
}

static void face_simple_set_line2(lv_obj_t * p_line, lv_point_t pts[2],
                                  lv_coord_t x1, lv_coord_t y1,
                                  lv_coord_t x2, lv_coord_t y2)
{
    lv_coord_t min_x = face_min2(x1, x2);
    lv_coord_t min_y = face_min2(y1, y2);
    lv_coord_t max_x = face_max2(x1, x2);
    lv_coord_t max_y = face_max2(y1, y2);

    pts[0].x = x1 - min_x;
    pts[0].y = y1 - min_y;
    pts[1].x = x2 - min_x;
    pts[1].y = y2 - min_y;

    lv_obj_set_pos(p_line, min_x, min_y);
    lv_obj_set_size(p_line, max_x - min_x + 1, max_y - min_y + 1);
    lv_line_set_points(p_line, pts, 2);
}

static void face_simple_set_line3(lv_obj_t * p_line, lv_point_t pts[3],
                                  lv_coord_t x1, lv_coord_t y1,
                                  lv_coord_t x2, lv_coord_t y2,
                                  lv_coord_t x3, lv_coord_t y3)
{
    lv_coord_t min_x = face_min3(x1, x2, x3);
    lv_coord_t min_y = face_min3(y1, y2, y3);
    lv_coord_t max_x = face_max3(x1, x2, x3);
    lv_coord_t max_y = face_max3(y1, y2, y3);

    pts[0].x = x1 - min_x;
    pts[0].y = y1 - min_y;
    pts[1].x = x2 - min_x;
    pts[1].y = y2 - min_y;
    pts[2].x = x3 - min_x;
    pts[2].y = y3 - min_y;

    lv_obj_set_pos(p_line, min_x, min_y);
    lv_obj_set_size(p_line, max_x - min_x + 1, max_y - min_y + 1);
    lv_line_set_points(p_line, pts, 3);
}

static void face_simple_set_eye_rect(lv_obj_t * p_eye,
                                     lv_coord_t x,
                                     lv_coord_t y,
                                     lv_coord_t w,
                                     lv_coord_t h)
{
    lv_obj_set_pos(p_eye, x, y);
    lv_obj_set_size(p_eye, w, h);
    lv_obj_set_style_radius(p_eye, LV_RADIUS_CIRCLE, 0);
}

static void face_simple_set_eyes_closed(void)
{
    lv_coord_t left_h  = lv_obj_get_height(g_face_simple.p_left_eye);
    lv_coord_t right_h = lv_obj_get_height(g_face_simple.p_right_eye);
    lv_coord_t new_h   = 4;

    lv_obj_set_y(g_face_simple.p_left_eye,
                 lv_obj_get_y(g_face_simple.p_left_eye) + (left_h - new_h) / 2);
    lv_obj_set_height(g_face_simple.p_left_eye, new_h);

    lv_obj_set_y(g_face_simple.p_right_eye,
                 lv_obj_get_y(g_face_simple.p_right_eye) + (right_h - new_h) / 2);
    lv_obj_set_height(g_face_simple.p_right_eye, new_h);
}

static void face_simple_hide_question(void)
{
    if (g_face_simple.p_cloud_container != NULL) {
        lv_obj_add_flag(g_face_simple.p_cloud_container, LV_OBJ_FLAG_HIDDEN);
    }
}

static void face_simple_show_question(void)
{
    if (g_face_simple.p_cloud_container != NULL) {
        lv_obj_clear_flag(g_face_simple.p_cloud_container, LV_OBJ_FLAG_HIDDEN);
    }
}

static void face_simple_apply_happy(void)
{
    lv_coord_t w = g_face_simple.width;
    lv_coord_t h = g_face_simple.height;
    lv_coord_t eye_w = face_pct(w, 18);
    lv_coord_t eye_h = face_pct(h, 10);
    lv_coord_t eye_y = face_pct(h, 33);

    face_simple_set_eye_rect(g_face_simple.p_left_eye,  face_pct(w, 24), eye_y, eye_w, eye_h);
    face_simple_set_eye_rect(g_face_simple.p_right_eye, face_pct(w, 58), eye_y, eye_w, eye_h);

    face_simple_set_line2(g_face_simple.p_left_brow, g_face_simple.left_brow_pts,
                          face_pct(w, 23), face_pct(h, 24),
                          face_pct(w, 42), face_pct(h, 26));

    face_simple_set_line2(g_face_simple.p_right_brow, g_face_simple.right_brow_pts,
                          face_pct(w, 58), face_pct(h, 26),
                          face_pct(w, 77), face_pct(h, 24));

    face_simple_set_line3(g_face_simple.p_mouth_line, g_face_simple.mouth_pts,
                          face_pct(w, 32), face_pct(h, 66),
                          face_pct(w, 50), face_pct(h, 75),
                          face_pct(w, 68), face_pct(h, 66));

    lv_obj_clear_flag(g_face_simple.p_mouth_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);
    face_simple_hide_question();
}

static void face_simple_apply_sad(void)
{
    lv_coord_t w = g_face_simple.width;
    lv_coord_t h = g_face_simple.height;
    lv_coord_t eye_w = face_pct(w, 18);
    lv_coord_t eye_h = face_pct(h, 8);
    lv_coord_t eye_y = face_pct(h, 36);

    face_simple_set_eye_rect(g_face_simple.p_left_eye,  face_pct(w, 24), eye_y, eye_w, eye_h);
    face_simple_set_eye_rect(g_face_simple.p_right_eye, face_pct(w, 58), eye_y, eye_w, eye_h);

    face_simple_set_line2(g_face_simple.p_left_brow, g_face_simple.left_brow_pts,
                          face_pct(w, 23), face_pct(h, 29),
                          face_pct(w, 42), face_pct(h, 24));

    face_simple_set_line2(g_face_simple.p_right_brow, g_face_simple.right_brow_pts,
                          face_pct(w, 58), face_pct(h, 24),
                          face_pct(w, 77), face_pct(h, 29));

    face_simple_set_line3(g_face_simple.p_mouth_line, g_face_simple.mouth_pts,
                          face_pct(w, 32), face_pct(h, 75),
                          face_pct(w, 50), face_pct(h, 66),
                          face_pct(w, 68), face_pct(h, 75));

    lv_obj_clear_flag(g_face_simple.p_mouth_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_pos(g_face_simple.p_tear, face_pct(w, 73), face_pct(h, 45));
    lv_obj_set_size(g_face_simple.p_tear, face_pct(w, 6), face_pct(h, 14));
    lv_obj_clear_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);
    face_simple_hide_question();
}

static void face_simple_apply_alert(void)
{
    lv_coord_t w = g_face_simple.width;
    lv_coord_t h = g_face_simple.height;
    lv_coord_t eye_w = face_pct(w, 16);
    lv_coord_t eye_h = face_pct(h, 16);
    lv_coord_t eye_y = face_pct(h, 31);

    face_simple_set_eye_rect(g_face_simple.p_left_eye,  face_pct(w, 25), eye_y, eye_w, eye_h);
    face_simple_set_eye_rect(g_face_simple.p_right_eye, face_pct(w, 59), eye_y, eye_w, eye_h);

    face_simple_set_line2(g_face_simple.p_left_brow, g_face_simple.left_brow_pts,
                          face_pct(w, 23), face_pct(h, 21),
                          face_pct(w, 42), face_pct(h, 22));

    face_simple_set_line2(g_face_simple.p_right_brow, g_face_simple.right_brow_pts,
                          face_pct(w, 58), face_pct(h, 22),
                          face_pct(w, 77), face_pct(h, 21));

    lv_obj_add_flag(g_face_simple.p_mouth_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_pos(g_face_simple.p_mouth_o, face_pct(w, 43), face_pct(h, 60));
    lv_obj_set_size(g_face_simple.p_mouth_o, face_pct(w, 14), face_pct(h, 18));
    face_simple_hide_question();
}

static void face_simple_apply_thinking(void)
{
    lv_coord_t w = g_face_simple.width;
    lv_coord_t h = g_face_simple.height;
    lv_coord_t eye_w = face_pct(w, 16);
    lv_coord_t eye_h = face_pct(h, 9);
    lv_coord_t eye_y = face_pct(h, 37);

    lv_coord_t brow_left_y1;
    lv_coord_t brow_left_y2;
    lv_coord_t brow_right_y1;
    lv_coord_t brow_right_y2;

    lv_coord_t mouth_y1;
    lv_coord_t mouth_y2;
    lv_coord_t mouth_y3;

    face_simple_set_eye_rect(g_face_simple.p_left_eye,  face_pct(w, 25), eye_y, eye_w, eye_h);
    face_simple_set_eye_rect(g_face_simple.p_right_eye, face_pct(w, 59), eye_y, eye_w, eye_h);

    brow_left_y1  = face_pct(h, 26);
    brow_left_y2  = face_pct(h, 17);
    brow_right_y1 = face_pct(h, 22);
    brow_right_y2 = face_pct(h, 22);

    if (1U == g_face_simple.thinking_brow_phase)
    {
        brow_left_y1  += 2;
        brow_left_y2  += 2;
        brow_right_y1 += 2;
        brow_right_y2 += 2;
    }

    face_simple_set_line2(g_face_simple.p_left_brow, g_face_simple.left_brow_pts,
                          face_pct(w, 22), brow_left_y1,
                          face_pct(w, 42), brow_left_y2);

    face_simple_set_line2(g_face_simple.p_right_brow, g_face_simple.right_brow_pts,
                          face_pct(w, 58), brow_right_y1,
                          face_pct(w, 78), brow_right_y2);

    mouth_y1 = face_pct(h, 67);
    mouth_y2 = face_pct(h, 69);
    mouth_y3 = face_pct(h, 69);

    if (0U == g_face_simple.thinking_mouth_phase)
    {
    }
    else if (1U == g_face_simple.thinking_mouth_phase)
    {
        mouth_y1 += 1;
        mouth_y2 += 0;
        mouth_y3 -= 1;
    }
    else
    {
        mouth_y1 -= 1;
        mouth_y2 += 1;
        mouth_y3 += 1;
    }

    face_simple_set_line3(g_face_simple.p_mouth_line, g_face_simple.mouth_pts,
                          face_pct(w, 40), mouth_y1,
                          face_pct(w, 53), mouth_y2,
                          face_pct(w, 66), mouth_y3);

    lv_obj_clear_flag(g_face_simple.p_mouth_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);

    face_simple_show_question();
}

static void face_simple_apply_air(void)
{
    lv_coord_t w = g_face_simple.width;
    lv_coord_t h = g_face_simple.height;

    lv_coord_t eye_w = face_pct(w, 14);
    lv_coord_t eye_h = face_pct(h, 14);
    lv_coord_t eye_y = face_pct(h, 33);

    face_simple_set_eye_rect(g_face_simple.p_left_eye,  face_pct(w, 25), eye_y, eye_w, eye_h);
    face_simple_set_eye_rect(g_face_simple.p_right_eye, face_pct(w, 59), eye_y, eye_w, eye_h);

    face_simple_set_line2(g_face_simple.p_left_brow, g_face_simple.left_brow_pts,
                          face_pct(w, 23), face_pct(h, 21),
                          face_pct(w, 42), face_pct(h, 24));

    face_simple_set_line2(g_face_simple.p_right_brow, g_face_simple.right_brow_pts,
                          face_pct(w, 58), face_pct(h, 24),
                          face_pct(w, 77), face_pct(h, 21));

    lv_obj_add_flag(g_face_simple.p_mouth_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);
    face_simple_hide_question();

    lv_obj_set_pos(g_face_simple.p_mouth_o, face_pct(w, 55), face_pct(h, 60));
    lv_obj_set_size(g_face_simple.p_mouth_o, face_pct(w, 10), face_pct(h, 10));
}

static void face_simple_apply_current_emotion(void)
{
    if (!g_face_simple.inited)
    {
        return;
    }

    switch (g_face_simple.emotion)
    {
        case FACE_SIMPLE_HAPPY:
            face_simple_apply_happy();
            break;

        case FACE_SIMPLE_SAD:
            face_simple_apply_sad();
            break;

        case FACE_SIMPLE_ALERT:
            face_simple_apply_alert();
            break;

        case FACE_SIMPLE_THINKING:
            face_simple_apply_thinking();
            break;

        case FACE_SIMPLE_AIR:
            face_simple_apply_air();
            break;

        default:
            face_simple_apply_happy();
            break;
    }
}

static void face_simple_stop_thinking_anim_timer(void)
{
    if (g_face_simple.p_thinking_anim_timer != NULL)
    {
        lv_timer_del(g_face_simple.p_thinking_anim_timer);
        g_face_simple.p_thinking_anim_timer = NULL;
    }

    g_face_simple.thinking_mouth_phase = 0U;
    g_face_simple.thinking_brow_phase  = 0U;
    g_face_simple.thinking_anim_tick   = 0U;
}

static void face_simple_thinking_anim_timer_cb(lv_timer_t * p_timer)
{
    FSP_PARAMETER_NOT_USED(p_timer);

    if (!g_face_simple.inited)
    {
        return;
    }

    if (FACE_SIMPLE_THINKING != g_face_simple.emotion)
    {
        return;
    }

    g_face_simple.thinking_anim_tick++;

    if (0U == g_face_simple.thinking_mouth_phase)
    {
        g_face_simple.thinking_mouth_phase = 1U;
    }
    else if (1U == g_face_simple.thinking_mouth_phase)
    {
        g_face_simple.thinking_mouth_phase = 2U;
    }
    else
    {
        g_face_simple.thinking_mouth_phase = 0U;
    }

    if ((g_face_simple.thinking_anim_tick % 4U) == 0U)
    {
        if (0U == g_face_simple.thinking_brow_phase)
        {
            g_face_simple.thinking_brow_phase = 1U;
        }
        else
        {
            g_face_simple.thinking_brow_phase = 0U;
        }
    }

    face_simple_apply_thinking();

    if (g_face_simple.is_blinking)
    {
        face_simple_set_eyes_closed();
    }
}

static void face_simple_start_thinking_anim_timer(void)
{
    face_simple_stop_thinking_anim_timer();

    g_face_simple.thinking_mouth_phase = 0U;
    g_face_simple.thinking_brow_phase  = 0U;
    g_face_simple.thinking_anim_tick   = 0U;

    g_face_simple.p_thinking_anim_timer = lv_timer_create(face_simple_thinking_anim_timer_cb,
                                                          180U,
                                                          NULL);
    if (NULL == g_face_simple.p_thinking_anim_timer)
    {
        uart_printf("[FACE ERROR] THINKING 动画定时器创建失败!\r\n");
        __BKPT(0);
    }
}

static void face_simple_update_runtime_timers(void)
{
    if (g_face_simple.auto_blink && (g_face_simple.p_blink_timer != NULL))
    {
        if (FACE_SIMPLE_THINKING == g_face_simple.emotion)
        {
            g_face_simple.blink_period_ms = 3200U;
        }
        else
        {
            g_face_simple.blink_period_ms = g_face_simple.normal_blink_period_ms;
        }

        if (!g_face_simple.is_blinking)
        {
            lv_timer_set_period(g_face_simple.p_blink_timer, g_face_simple.blink_period_ms);
        }
    }

    if (FACE_SIMPLE_THINKING == g_face_simple.emotion)
    {
        if (NULL == g_face_simple.p_thinking_anim_timer)
        {
            face_simple_start_thinking_anim_timer();
        }
    }
    else
    {
        face_simple_stop_thinking_anim_timer();
    }
}

static void face_simple_manual_blink_restore_cb(lv_timer_t * p_timer)
{
    face_simple_apply_current_emotion();
    g_face_simple.is_blinking = false;
    lv_timer_del(p_timer);
}

static void face_simple_blink_timer_cb(lv_timer_t * p_timer)
{
    if (!g_face_simple.inited)
    {
        return;
    }

    if (!g_face_simple.is_blinking)
    {
        g_face_simple.is_blinking = true;
        face_simple_set_eyes_closed();
        lv_timer_set_period(p_timer, 120U);
    }
    else
    {
        g_face_simple.is_blinking = false;
        face_simple_apply_current_emotion();
        lv_timer_set_period(p_timer, g_face_simple.blink_period_ms);
    }
}

/* 辅助函数：快速创建一个纯色圆块，用于组合云朵（底层作边框，顶层作填充即可解决重叠问题） */
static lv_obj_t * face_simple_create_cloud_part_base(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_color_t color)
{
    lv_obj_t * p_part = lv_obj_create(parent);
    lv_obj_remove_style_all(p_part);
    lv_obj_set_style_bg_opa(p_part, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(p_part, color, 0);
    lv_obj_set_style_radius(p_part, LV_RADIUS_CIRCLE, 0);         // 圆角拉满
    lv_obj_set_pos(p_part, x, y);
    lv_obj_set_size(p_part, w, h);
    return p_part;
}

static void face_simple_create_objects(void)
{
    g_face_simple.p_container   = lv_obj_create(g_face_simple.p_parent);
    g_face_simple.p_left_eye    = lv_obj_create(g_face_simple.p_container);
    g_face_simple.p_right_eye   = lv_obj_create(g_face_simple.p_container);
    g_face_simple.p_left_brow   = lv_line_create(g_face_simple.p_container);
    g_face_simple.p_right_brow  = lv_line_create(g_face_simple.p_container);
    g_face_simple.p_mouth_line  = lv_line_create(g_face_simple.p_container);
    g_face_simple.p_mouth_o     = lv_obj_create(g_face_simple.p_container);
    g_face_simple.p_tear        = lv_obj_create(g_face_simple.p_container);

    if ((NULL == g_face_simple.p_container)   ||
        (NULL == g_face_simple.p_left_eye)    ||
        (NULL == g_face_simple.p_right_eye)   ||
        (NULL == g_face_simple.p_left_brow)   ||
        (NULL == g_face_simple.p_right_brow)  ||
        (NULL == g_face_simple.p_mouth_line)  ||
        (NULL == g_face_simple.p_mouth_o)     ||
        (NULL == g_face_simple.p_tear))
    {
        uart_printf("[FACE ERROR] LVGL 对象创建失败!\r\n");
        __BKPT(0);
    }

    lv_obj_remove_style_all(g_face_simple.p_container);
    lv_obj_set_size(g_face_simple.p_container, g_face_simple.width, g_face_simple.height);
    lv_obj_set_pos(g_face_simple.p_container, 0, 0);
    lv_obj_clear_flag(g_face_simple.p_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_face_simple.p_container, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_face_simple.p_container, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_face_simple.p_container, lv_color_hex(0xFFF1B8), 0);
    lv_obj_set_style_border_width(g_face_simple.p_container, 2, 0);
    lv_obj_set_style_border_color(g_face_simple.p_container, lv_color_hex(0x404040), 0);
    lv_obj_set_style_pad_all(g_face_simple.p_container, 0, 0);

    lv_obj_remove_style_all(g_face_simple.p_left_eye);
    lv_obj_remove_style_all(g_face_simple.p_right_eye);
    lv_obj_set_style_bg_opa(g_face_simple.p_left_eye, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(g_face_simple.p_right_eye, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_face_simple.p_left_eye, lv_color_hex(0x202020), 0);
    lv_obj_set_style_bg_color(g_face_simple.p_right_eye, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(g_face_simple.p_left_eye, 0, 0);
    lv_obj_set_style_border_width(g_face_simple.p_right_eye, 0, 0);

    lv_obj_set_style_line_width(g_face_simple.p_left_brow, 4, 0);
    lv_obj_set_style_line_width(g_face_simple.p_right_brow, 4, 0);
    lv_obj_set_style_line_color(g_face_simple.p_left_brow, lv_color_hex(0x202020), 0);
    lv_obj_set_style_line_color(g_face_simple.p_right_brow, lv_color_hex(0x202020), 0);
    lv_obj_set_style_line_rounded(g_face_simple.p_left_brow, true, 0);
    lv_obj_set_style_line_rounded(g_face_simple.p_right_brow, true, 0);

    lv_obj_set_style_line_width(g_face_simple.p_mouth_line, 5, 0);
    lv_obj_set_style_line_color(g_face_simple.p_mouth_line, lv_color_hex(0x7A2F2F), 0);
    lv_obj_set_style_line_rounded(g_face_simple.p_mouth_line, true, 0);

    lv_obj_remove_style_all(g_face_simple.p_mouth_o);
    lv_obj_set_style_bg_opa(g_face_simple.p_mouth_o, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_face_simple.p_mouth_o, lv_color_hex(0x7A2F2F), 0);
    lv_obj_set_style_radius(g_face_simple.p_mouth_o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(g_face_simple.p_mouth_o, 0, 0);
    lv_obj_add_flag(g_face_simple.p_mouth_o, LV_OBJ_FLAG_HIDDEN);

    lv_obj_remove_style_all(g_face_simple.p_tear);
    lv_obj_set_style_bg_opa(g_face_simple.p_tear, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_face_simple.p_tear, lv_color_hex(0x5AB7FF), 0);
    lv_obj_set_style_radius(g_face_simple.p_tear, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(g_face_simple.p_tear, 0, 0);
    lv_obj_add_flag(g_face_simple.p_tear, LV_OBJ_FLAG_HIDDEN);

    /* ================= 新增：联想云朵及问号组合 ================= */
    g_face_simple.p_cloud_container = lv_obj_create(g_face_simple.p_parent);
    lv_obj_remove_style_all(g_face_simple.p_cloud_container);

    // 云朵容器大小按脸部比例动态生成 (宽度 70%, 高度 50%)
    lv_coord_t cloud_w = face_pct(g_face_simple.width, 70);
    lv_coord_t cloud_h = face_pct(g_face_simple.height, 50);
    lv_obj_set_size(g_face_simple.p_cloud_container, cloud_w, cloud_h);

    lv_color_t color_border = lv_color_hex(0x000000);
    lv_color_t color_fill   = lv_color_hex(0xD6EAF8);
    lv_coord_t bw = 2; // 边框粗细

    // 1. 先绘制作为黑色粗边框的底层 (主云朵 + 下面的两个小尾巴，全部先画黑底)
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 25) - bw, face_pct(cloud_h, 20) - bw, face_pct(cloud_w, 35) + bw * 2, face_pct(cloud_h, 45) + bw * 2, color_border);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 60) - bw, face_pct(cloud_h, 20) - bw, face_pct(cloud_w, 35) + bw * 2, face_pct(cloud_h, 45) + bw * 2, color_border);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 40) - bw, face_pct(cloud_h, 5) - bw,  face_pct(cloud_w, 45) + bw * 2, face_pct(cloud_h, 55) + bw * 2, color_border);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 35) - bw, face_pct(cloud_h, 30) - bw, face_pct(cloud_w, 40) + bw * 2, face_pct(cloud_h, 40) + bw * 2, color_border);
    /* 小尾巴黑底 */
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 5) - bw,  face_pct(cloud_h, 80) - bw, face_pct(cloud_w, 10) + bw * 2, face_pct(cloud_h, 14) + bw * 2, color_border);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 20) - bw, face_pct(cloud_h, 60) - bw, face_pct(cloud_w, 16) + bw * 2, face_pct(cloud_h, 22) + bw * 2, color_border);

    // 2. 绘制实际颜色(浅蓝)的填充顶层，盖住刚才所有的黑底内部交叠处
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 25), face_pct(cloud_h, 20), face_pct(cloud_w, 35), face_pct(cloud_h, 45), color_fill);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 60), face_pct(cloud_h, 20), face_pct(cloud_w, 35), face_pct(cloud_h, 45), color_fill);
    lv_obj_t * p_central_cloud_part = face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 40), face_pct(cloud_h, 5), face_pct(cloud_w, 45), face_pct(cloud_h, 55), color_fill);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 35), face_pct(cloud_h, 30), face_pct(cloud_w, 40), face_pct(cloud_h, 40), color_fill);
    /* 小尾巴蓝层 */
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 5),  face_pct(cloud_h, 80), face_pct(cloud_w, 10), face_pct(cloud_h, 14), color_fill);
    face_simple_create_cloud_part_base(g_face_simple.p_cloud_container, face_pct(cloud_w, 20), face_pct(cloud_h, 60), face_pct(cloud_w, 16), face_pct(cloud_h, 22), color_fill);

    // 3. 问号标签：将它直接挂载在 p_cloud_container 下，作为最后创建的子对象，确保处于最顶层
    g_face_simple.p_question_label = lv_label_create(g_face_simple.p_cloud_container);
    lv_label_set_text(g_face_simple.p_question_label, "?");
    lv_obj_set_style_text_font(g_face_simple.p_question_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(g_face_simple.p_question_label, lv_color_hex(0xE07000), 0); // 橙色问号

    // 将问号精准对齐到刚保存的“中间大蓝块”的正中央
    lv_obj_align_to(g_face_simple.p_question_label, p_central_cloud_part, LV_ALIGN_CENTER, 0, 0);

    // 初始状态先隐藏云朵
    face_simple_hide_question();

    /* ================= 底部文字说明 ================= */
    g_face_simple.p_status_label = lv_label_create(g_face_simple.p_parent);
    lv_obj_set_style_text_font(g_face_simple.p_status_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_face_simple.p_status_label, lv_color_hex(0x404040), 0);
    lv_label_set_text(g_face_simple.p_status_label, "Ready");
    lv_obj_align_to(g_face_simple.p_status_label, g_face_simple.p_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

fsp_err_t face_simple_init(face_simple_cfg_t const * p_cfg)
{
    face_simple_cfg_t cfg;

    if (NULL != p_cfg)
    {
        cfg = *p_cfg;
    }
    else
    {
        cfg.p_parent        = NULL;
        cfg.width           = 160;
        cfg.height          = 160;
        cfg.pos_x           = 40;
        cfg.pos_y           = 60;
        cfg.auto_blink      = true;
        cfg.blink_period_ms = 2500U;
    }

    if (g_face_simple.inited)
    {
        face_simple_deinit();
    }

    if (cfg.width <= 0)
    {
        cfg.width = 160;
    }

    if (cfg.height <= 0)
    {
        cfg.height = 160;
    }

    if (0U == cfg.blink_period_ms)
    {
        cfg.blink_period_ms = 2500U;
    }

    if (NULL == cfg.p_parent)
    {
        cfg.p_parent = lv_scr_act();
    }

    memset(&g_face_simple, 0, sizeof(g_face_simple));

    g_face_simple.p_parent                = cfg.p_parent;
    g_face_simple.width                   = cfg.width;
    g_face_simple.height                  = cfg.height;
    g_face_simple.auto_blink              = cfg.auto_blink;
    g_face_simple.blink_period_ms         = cfg.blink_period_ms;
    g_face_simple.normal_blink_period_ms  = cfg.blink_period_ms;
    g_face_simple.emotion                 = FACE_SIMPLE_HAPPY;

    face_simple_create_objects();
    lv_obj_set_pos(g_face_simple.p_container, cfg.pos_x, cfg.pos_y);

    // 初始化时对齐一次云朵容器到脸部右上角
    if (g_face_simple.p_cloud_container != NULL) {
        /* 【修改偏移量】正数X偏置，让云朵向右靠一点，解决太偏左的问题 */
        lv_obj_align_to(g_face_simple.p_cloud_container, g_face_simple.p_container, LV_ALIGN_OUT_TOP_RIGHT, face_pct(cfg.width, 10), face_pct(cfg.height, 20));
    }

    g_face_simple.inited = true;
    face_simple_apply_current_emotion();

    if (g_face_simple.auto_blink)
    {
        g_face_simple.p_blink_timer = lv_timer_create(face_simple_blink_timer_cb,
                                                      g_face_simple.blink_period_ms,
                                                      NULL);
        if (NULL == g_face_simple.p_blink_timer)
        {
            uart_printf("[FACE ERROR] 自动眨眼定时器创建失败!\r\n");
            __BKPT(0);
        }
    }

    return FSP_SUCCESS;
}

void face_simple_deinit(void)
{
    face_simple_stop_thinking_anim_timer();

    if (g_face_simple.p_blink_timer != NULL)
    {
        lv_timer_del(g_face_simple.p_blink_timer);
        g_face_simple.p_blink_timer = NULL;
    }

    if (g_face_simple.p_container != NULL)
    {
        lv_obj_del(g_face_simple.p_container);
        g_face_simple.p_container = NULL;
    }

    if (g_face_simple.p_status_label != NULL)
    {
        lv_obj_del(g_face_simple.p_status_label);
        g_face_simple.p_status_label = NULL;
    }

    /* 清理云朵 */
    if (g_face_simple.p_cloud_container != NULL)
    {
        lv_obj_del(g_face_simple.p_cloud_container);
        g_face_simple.p_cloud_container = NULL;
    }

    memset(&g_face_simple, 0, sizeof(g_face_simple));
}

void face_simple_set_emotion(face_simple_emotion_t emotion)
{
    if (!g_face_simple.inited)
    {
        return;
    }

    if (emotion >= FACE_SIMPLE_EMOTION_MAX)
    {
        return;
    }

    g_face_simple.emotion = emotion;
    face_simple_update_runtime_timers();

    if (!g_face_simple.is_blinking)
    {
        face_simple_apply_current_emotion();
    }

    /* 根据表情更新底部文字 */
    if (g_face_simple.p_status_label != NULL)
    {
        switch (emotion)
        {
            case FACE_SIMPLE_HAPPY:
                lv_label_set_text(g_face_simple.p_status_label, "Perfume");   // 香水
                break;
            case FACE_SIMPLE_SAD:
                lv_label_set_text(g_face_simple.p_status_label, "Vinegar");   // 醋
                break;
            case FACE_SIMPLE_ALERT:
                lv_label_set_text(g_face_simple.p_status_label, "Alcohol");   // 酒精
                break;
            case FACE_SIMPLE_THINKING:
                lv_label_set_text(g_face_simple.p_status_label, "Detecting..."); // 检测中/思考中
                break;
            case FACE_SIMPLE_AIR:
                lv_label_set_text(g_face_simple.p_status_label, "Air");       // 空气
                break;
            default:
                lv_label_set_text(g_face_simple.p_status_label, "");
                break;
        }

        // 重新居中对齐
        lv_obj_align_to(g_face_simple.p_status_label, g_face_simple.p_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }
}

face_simple_emotion_t face_simple_get_emotion(void)
{
    return g_face_simple.emotion;
}

void face_simple_set_position(lv_coord_t x, lv_coord_t y)
{
    if (!g_face_simple.inited)
    {
        return;
    }

    lv_obj_set_pos(g_face_simple.p_container, x, y);

    // 每次移动脸，都让文字保持在正下方
    if (g_face_simple.p_status_label != NULL) {
        lv_obj_align_to(g_face_simple.p_status_label, g_face_simple.p_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }

    // 让云朵也跟着移动
    if (g_face_simple.p_cloud_container != NULL) {
        /* 【修改偏移量】保持和 init 里的参数一致 */
        lv_obj_align_to(g_face_simple.p_cloud_container, g_face_simple.p_container, LV_ALIGN_OUT_TOP_RIGHT, face_pct(g_face_simple.width, 10), face_pct(g_face_simple.height, 20));
    }
}

lv_obj_t * face_simple_get_container(void)
{
    return g_face_simple.p_container;
}

void face_simple_trigger_blink(void)
{
    lv_timer_t * p_restore_timer;

    if (!g_face_simple.inited)
    {
        return;
    }

    if (g_face_simple.is_blinking)
    {
        return;
    }

    g_face_simple.is_blinking = true;
    face_simple_set_eyes_closed();

    p_restore_timer = lv_timer_create(face_simple_manual_blink_restore_cb, 120U, NULL);
    if (NULL == p_restore_timer)
    {
        uart_printf("[FACE ERROR] 手动眨眼恢复定时器创建失败!\r\n");
        __BKPT(0);
    }
}
