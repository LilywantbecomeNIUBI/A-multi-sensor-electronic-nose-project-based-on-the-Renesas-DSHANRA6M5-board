#include "sensor_uart.h"

#define SENSOR_UART_CSV_LINE_BUF_SIZE    (768U)

typedef enum e_sensor_uart_mode
{
    SENSOR_UART_MODE_HUMAN = 0,
    SENSOR_UART_MODE_CSV
} sensor_uart_mode_t;

static sensor_uart_mode_t g_sensor_uart_mode = SENSOR_UART_MODE_HUMAN;
static bool               g_sensor_uart_csv_header_printed = false;

static fsp_err_t sensor_uart_print_mq(sensor_hub_mq_snapshot_t const * p_mq)
{
    char line[192];
    uint32_t pos = 0U;
    uint32_t ch;
    fsp_err_t err;

    if (NULL == p_mq)
    {
        return FSP_ERR_ASSERTION;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[MQ] frame=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->frame_complete ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " acq_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->acquire_err);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " sm_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->process_err);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " state=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) p_mq->state);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " quality=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) p_mq->quality);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " flags=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->flags);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " event=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) p_mq->event);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    err = AppDebug_WriteString(line);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    line[0] = '\0';
    pos = 0U;
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[MQ] metrics_valid=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->metrics_valid ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " baseline_ready=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->baseline_ready ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " responding=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->responding ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " recovered=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->recovered ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " warmup=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->warmup_elapsed_frames);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " window=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->stable_window_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " update=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_mq->update_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " metrics_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->last_metrics_err);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    err = AppDebug_WriteString(line);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        line[0] = '\0';
        pos = 0U;

        pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[MQ-CH");
        pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, ch);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, "] raw=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, (int32_t) p_mq->raw[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " mV=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->mv[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " base=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->baseline_mv[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " delta=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->delta_mv[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " ratio_pm=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->ratio_permille[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " peak=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_mq->peak_delta_mv[ch]);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
        FSP_PARAMETER_NOT_USED(pos);

        err = AppDebug_WriteString(line);
        if (FSP_SUCCESS != err)
        {
            return err;
        }
    }

    return FSP_SUCCESS;
}

static fsp_err_t sensor_uart_print_env(sensor_hub_env_snapshot_t const * p_env)
{
    char line[192];
    uint32_t pos = 0U;

    if (NULL == p_env)
    {
        return FSP_ERR_ASSERTION;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[ENV] valid=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_env->valid ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " stage=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) p_env->last_stage);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_env->service_err);

    if (p_env->valid)
    {
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " T=");
        pos = AppDebug_BufAppendFloat1(line, sizeof(line), pos, p_env->temperature_c);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, "C RH=");
        pos = AppDebug_BufAppendFloat1(line, sizeof(line), pos, p_env->humidity_rh);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, "% sraw=");
        pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_env->sraw);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " voc=");
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_env->voc_index);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " blackout=");
        pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_env->blackout ? 1U : 0U);
        pos = AppDebug_BufAppendString(line, sizeof(line), pos, " sample=");
        pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_env->sample_count);
    }

    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    return AppDebug_WriteString(line);
}

static fsp_err_t sensor_uart_print_i2c(sensor_hub_record_t const * p_record)
{
    char line[160];
    uint32_t pos = 0U;

    if (NULL == p_record)
    {
        return FSP_ERR_ASSERTION;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[I2C] cb=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->i2c_callback_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " timeout=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->i2c_timeout_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " recover=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->i2c_recover_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " aborted=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->i2c_aborted ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " last_event=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) p_record->i2c_last_event);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    return AppDebug_WriteString(line);
}

static uint32_t sensor_uart_csv_append_comma(char * p_buf, uint32_t buf_size, uint32_t pos)
{
    return AppDebug_BufAppendChar(p_buf, buf_size, pos, ',');
}

static uint32_t sensor_uart_csv_append_bool(char * p_buf, uint32_t buf_size, uint32_t pos, bool value)
{
    return AppDebug_BufAppendUInt32(p_buf, buf_size, pos, value ? 1U : 0U);
}

static char const * sensor_uart_get_csv_mq_name(uint32_t ch)
{
    switch (ch)
    {
        case 0U:
        {
            return "mq135";
        }

        case 1U:
        {
            return "mq138";
        }

        case 2U:
        {
            return "mq2";
        }

        case 3U:
        {
            return "mq3";
        }

        default:
        {
            return "mq_unknown";
        }
    }
}

static uint32_t sensor_uart_csv_append_feature_suffix(char * p_buf,
                                                      uint32_t buf_size,
                                                      uint32_t pos,
                                                      char const * p_feature,
                                                      char const * p_suffix)
{
    pos = AppDebug_BufAppendString(p_buf, buf_size, pos, p_feature);
    pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, '_');
    pos = AppDebug_BufAppendString(p_buf, buf_size, pos, p_suffix);
    return pos;
}

static fsp_err_t sensor_uart_print_csv_header(void)
{
    char line[SENSOR_UART_CSV_LINE_BUF_SIZE];
    uint32_t pos = 0U;
    uint32_t ch;
    char const * p_name;

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "sequence");
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "atomizer_on");
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "env_temperature_c");
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "env_humidity_rh");
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "env_voc_index");

    /*
     * 对应硬件功能：
     * - 把 ADS1115#1 的 4 路 MQ 通道映射为正式 CSV 的传感器名表头
     *
     * 依赖资料：
     * - 当前项目确认的通道映射：A0=MQ135, A1=MQ138, A2=MQ2, A3=MQ3
     * - ADS1115_CHANNEL_MAX 对应第一块 ADS1115 的 4 路输入
     *
     * 前提假设：
     * - 正式采集阶段仅导出第一块 ADS1115 的 4 路 MQ 特征
     */
    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_name = sensor_uart_get_csv_mq_name(ch);

        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = sensor_uart_csv_append_feature_suffix(line, sizeof(line), pos, p_name, "mv");
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = sensor_uart_csv_append_feature_suffix(line, sizeof(line), pos, p_name, "baseline_mv");
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = sensor_uart_csv_append_feature_suffix(line, sizeof(line), pos, p_name, "delta_mv");
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = sensor_uart_csv_append_feature_suffix(line, sizeof(line), pos, p_name, "ratio_pm");
    }

    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);
    return AppDebug_WriteString(line);
}

static fsp_err_t sensor_uart_ensure_csv_header(void)
{
    fsp_err_t err;

    if (g_sensor_uart_csv_header_printed)
    {
        return FSP_SUCCESS;
    }

    err = sensor_uart_print_csv_header();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_sensor_uart_csv_header_printed = true;
    return FSP_SUCCESS;
}

static fsp_err_t sensor_uart_print_csv_record(sensor_hub_record_t const * p_record)
{
    char line[SENSOR_UART_CSV_LINE_BUF_SIZE];
    uint32_t pos = 0U;
    uint32_t ch;

    if (NULL == p_record)
    {
        return FSP_ERR_ASSERTION;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->sequence);
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = sensor_uart_csv_append_bool(line, sizeof(line), pos, p_record->atomizer_on);
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendFloat1(line, sizeof(line), pos, p_record->env.temperature_c);
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendFloat1(line, sizeof(line), pos, p_record->env.humidity_rh);
    pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_record->env.voc_index);

    /*
     * 对应硬件功能：
     * - 导出第一块 ADS1115 上 4 路 MQ 的正式采样特征值
     *
     * 依赖资料：
     * - MQ 状态机已经写回 p_record->mq.mv / baseline_mv / delta_mv / ratio_permille
     * - 当前通道映射：A0=MQ135, A1=MQ138, A2=MQ2, A3=MQ3
     *
     * 前提假设：
     * - 正式采样态不再输出调试错误码、状态位和 I2C 诊断字段
     */
    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_record->mq.mv[ch]);
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_record->mq.baseline_mv[ch]);
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_record->mq.delta_mv[ch]);
        pos = sensor_uart_csv_append_comma(line, sizeof(line), pos);
        pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_record->mq.ratio_permille[ch]);
    }

    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);
    return AppDebug_WriteString(line);
}

fsp_err_t SensorUart_Init(void)
{
    g_sensor_uart_mode = SENSOR_UART_MODE_HUMAN;
    g_sensor_uart_csv_header_printed = false;
    return AppDebug_UartOpen();
}

fsp_err_t SensorUart_EnableHumanMode(void)
{
    if (SENSOR_UART_MODE_HUMAN != g_sensor_uart_mode)
    {
        g_sensor_uart_mode = SENSOR_UART_MODE_HUMAN;
        g_sensor_uart_csv_header_printed = false;
    }

    return FSP_SUCCESS;
}

fsp_err_t SensorUart_EnableCsvMode(void)
{
    if (SENSOR_UART_MODE_CSV != g_sensor_uart_mode)
    {
        g_sensor_uart_mode = SENSOR_UART_MODE_CSV;
        g_sensor_uart_csv_header_printed = false;
        return sensor_uart_ensure_csv_header();
    }

    return FSP_SUCCESS;
}

bool SensorUart_IsCsvMode(void)
{
    return (SENSOR_UART_MODE_CSV == g_sensor_uart_mode);
}

fsp_err_t SensorUart_PrintBanner(void)
{
    fsp_err_t err;

    if (SensorUart_IsCsvMode())
    {
        return FSP_SUCCESS;
    }

    err = AppDebug_WriteString("\r\nSCI7 open ok\r\n");
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return AppDebug_WriteString("sensor hub unified text output start\r\n");
}

fsp_err_t SensorUart_PrintInitResult(sensor_hub_t const * p_hub, int32_t hub_err)
{
    char line[192];
    uint32_t pos = 0U;
    uint32_t i;
    fsp_err_t err;

    if (NULL == p_hub)
    {
        return FSP_ERR_ASSERTION;
    }

    if (SensorUart_IsCsvMode())
    {
        return FSP_SUCCESS;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[INIT] hub_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, hub_err);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " mq_init_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_hub->last_mq_init_error);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " env_init_err=");
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, p_hub->last_env_init_error);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " ads_addr=0x");
    pos = AppDebug_BufAppendHex8(line, sizeof(line), pos, ADS1115_GetAddress());
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    err = AppDebug_WriteString(line);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    line[0] = '\0';
    pos = 0U;
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[SGP40] addr=0x");
    pos = AppDebug_BufAppendHex8(line, sizeof(line), pos, sgp40_get_configured_address());
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, " drv=");
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, sgp40_get_driver_version());
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n[SGP40] serial=");

    for (i = 0U; i < (uint32_t) SGP40_SERIAL_ID_NUM_BYTES; i++)
    {
        pos = AppDebug_BufAppendHex8(line, sizeof(line), pos, p_hub->env_service.sgp40_voc.serial_id[i]);
        if (i < ((uint32_t) SGP40_SERIAL_ID_NUM_BYTES - 1U))
        {
            pos = AppDebug_BufAppendChar(line, sizeof(line), pos, '-');
        }
    }

    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    err = AppDebug_WriteString(line);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return AppDebug_PrintI2CDiag();
}

fsp_err_t SensorUart_PrintRecord(sensor_hub_record_t const * p_record)
{
    char line[64];
    uint32_t pos = 0U;
    fsp_err_t err;

    if (NULL == p_record)
    {
        return FSP_ERR_ASSERTION;
    }

    if (SensorUart_IsCsvMode())
    {
        err = sensor_uart_ensure_csv_header();
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        return sensor_uart_print_csv_record(p_record);
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[FRAME] seq=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, p_record->sequence);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    err = AppDebug_WriteString(line);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = sensor_uart_print_mq(&p_record->mq);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = sensor_uart_print_env(&p_record->env);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return sensor_uart_print_i2c(p_record);
}
