#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".pc_mic_recoder.data.bss")
#pragma data_seg(".pc_mic_recoder.data")
#pragma const_seg(".pc_mic_recoder.text.const")
#pragma code_seg(".pc_mic_recoder.text")
#endif
#include "pc_mic_recoder.h"
#include "app_config.h"
#include "media/includes.h"
#include "jlstream.h"
#include "uac_stream.h"
#include "asm/dac.h"

#if TCFG_USB_SLAVE_AUDIO_MIC_ENABLE

extern struct audio_dac_hdl dac_hdl;

struct pc_mic_recoder {
    struct jlstream *stream;
    u8 open_flag;
};
static struct pc_mic_recoder *g_pc_mic_recoder = NULL;

//防止通过消息队列打开、关闭或者重启recoder被多次调用
static u8 recoder_wait_open_flag = 0;
static u8 recoder_wait_close_flag = 0;
static u8 recoder_wait_restart_flag = 0;

static void pc_mic_recoder_callback(void *private_data, int event)
{
    struct pc_mic_recoder *recoder = g_pc_mic_recoder;
    struct jlstream *stream = (struct jlstream *)private_data;

    switch (event) {
    case STREAM_EVENT_START:
        break;
    }
}

int pc_mic_recoder_open(void)
{
    int err;
    struct pc_mic_recoder *recoder = NULL;
    recoder_wait_open_flag = 0;
    if (g_pc_mic_recoder) {
        r_printf("## %s, pc mic recoder is busy!\n", __func__);
        return -EBUSY;
    }
    u16 uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"pc_mic");
    if (uuid == 0) {
        return -EFAULT;
    }
    recoder = zalloc(sizeof(*recoder));
    if (!recoder) {
        return -ENOMEM;
    }
    recoder->stream = jlstream_pipeline_parse(uuid, NODE_UUID_ADC);

    if (!recoder->stream) {
        err = -ENOMEM;
        goto __exit0;
    }

    //设置ADC的中断点数
    err = jlstream_node_ioctl(recoder->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PRIV_FMT, 256);
    if (err) {
        goto __exit1;
    }
    u16 ref_sr = audio_dac_get_sample_rate(&dac_hdl);
    u16 node_uuid = 0;
#if   (defined TCFG_AUDIO_CVP_SMS_ANS_MODE)
    node_uuid = NODE_UUID_CVP_SMS_ANS;
#elif (defined TCFG_AUDIO_CVP_SMS_DNS_MODE)
    node_uuid = NODE_UUID_CVP_SMS_DNS;
#elif (defined TCFG_AUDIO_CVP_DMS_ANS_MODE)
    node_uuid = NODE_UUID_CVP_DMS_ANS;
#elif (defined TCFG_AUDIO_CVP_DMS_DNS_MODE)
    node_uuid = NODE_UUID_CVP_DMS_DNS;
#elif (defined TCFG_AUDIO_CVP_DMS_FLEXIBLE_ANS_MODE)
    node_uuid = NODE_UUID_CVP_DMS_FLEXIBLE_ANS;
#elif (defined TCFG_AUDIO_CVP_DMS_FLEXIBLE_DNS_MODE)
    node_uuid = NODE_UUID_CVP_DMS_FLEXIBLE_DNS;
#elif (defined TCFG_AUDIO_CVP_3MIC_MODE)
    node_uuid = NODE_UUID_CVP_3MIC;
#elif (defined TCFG_AUDIO_CVP_DEVELOP_ENABLE)
    node_uuid = NODE_UUID_CVP_DEVELOP;
#endif
    //根据回音消除的类型，将配置传递到对应的节点
    if (node_uuid) {
        err = jlstream_node_ioctl(recoder->stream, node_uuid, NODE_IOC_SET_FMT, (int)ref_sr);
    }
    if (err && (err != -ENOENT)) {	//兼容没有cvp节点的情况
        goto __exit1;
    }
    jlstream_set_callback(recoder->stream, recoder->stream, pc_mic_recoder_callback);
    jlstream_set_scene(recoder->stream, STREAM_SCENE_PC_MIC);
    err = jlstream_start(recoder->stream);
    if (err) {
        goto __exit1;
    }

    recoder->open_flag = 1;
    g_pc_mic_recoder = recoder;

    return 0;

__exit1:
    jlstream_release(recoder->stream);
__exit0:
    free(recoder);
    return err;
}


void pc_mic_recoder_close(void)
{
    struct pc_mic_recoder *recoder = g_pc_mic_recoder;

    recoder_wait_close_flag = 0;
    if (!recoder) {
        return;
    }
    if (pc_mic_get_node_state()) {
        //pc_mic_get_node_state 返回1代表有收到数据
        jlstream_stop(recoder->stream, 0);
    }
    jlstream_release(recoder->stream);
    recoder->open_flag = 0;

    free(recoder);
    g_pc_mic_recoder = NULL;

    jlstream_event_notify(STREAM_EVENT_CLOSE_RECODER, (int)"pc_mic");
}

//重启pc mic
static void pc_mic_recoder_restart(void)
{
    if (g_pc_mic_recoder) {
        if (g_pc_mic_recoder->open_flag) {
            pc_mic_recoder_close();
        }
    }
    pc_mic_recoder_open();
    recoder_wait_restart_flag = 0;
}

int pc_mic_recoder_open_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    y_printf(">> Case : USB_MIC_REC_OPEN 111\n");
    if (recoder_wait_open_flag == 0) {
        y_printf(">> Case : USB_MIC_REC_OPEN 222\n");
        msg[0] = (int)pc_mic_recoder_open;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        recoder_wait_open_flag = 1;
    }
    return ret;
}

int pc_mic_recoder_close_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    if (recoder_wait_close_flag == 0) {
        msg[0] = (int)pc_mic_recoder_close;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        recoder_wait_close_flag = 1;
    }
    return ret;
}

int pc_mic_recoder_restart_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    if (recoder_wait_restart_flag == 0) {
        msg[0] = (int)pc_mic_recoder_restart;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        recoder_wait_restart_flag = 1;
    }
    return ret;
}

#endif


