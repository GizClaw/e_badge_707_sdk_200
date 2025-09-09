#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".pc_spk_file.data.bss")
#pragma data_seg(".pc_spk_file.data")
#pragma const_seg(".pc_spk_file.text.const")
#pragma code_seg(".pc_spk_file.text")
#endif
#include "source_node.h"
#include "media/audio_splicing.h"
#include "audio_config.h"
#include "jlstream.h"
#include "pc_spk_file.h"
#include "app_config.h"
#include "effects/effects_adj.h"
#include "gpio_config.h"
#include "sync/audio_clk_sync.h"
#include "clock_manager/clock_manager.h"
#include "spinlock.h"
#include "circular_buf.h"
#include "pc_spk_player.h"
#include "uac_stream.h"

#if  TCFG_USB_SLAVE_AUDIO_SPK_ENABLE

/* #define PC_SPK_CACHE_BUF_LEN   (1024 * 2) */
//足够缓存20ms的数据
#define PC_SPK_CACHE_BUF_LEN   ((SPK_AUDIO_RATE/1000) * 4 * 20)

/* PC SPK 在线检测 */
#define PC_SPK_ONLINE_DET_EN   1
#define PC_SPK_ONLINE_DET_TIME 50

struct pc_spk_file_hdl {
    void *source_node;
    struct stream_node *node;
    struct stream_snode *snode;
    cbuffer_t spk_cache_cbuffer;
#if PC_SPK_ONLINE_DET_EN
    u32 irq_cnt;		//进中断++，用来给定时器判断是否中断没有起
    u16 det_timer_id;
#endif
    int sr;
    u8 *cache_buf;
    u8 start;
    u8 data_run;
};
static struct pc_spk_file_hdl *pc_spk = NULL;

struct pc_spk_fmt_t {
    u8 init;
    u8 channel;
    u8 bit;
    u32 sample_rate;
};
struct pc_spk_fmt_t pc_spk_fmt = {
    .init = 0,
    .channel = SPK_CHANNEL,
    .bit = SPK_AUDIO_RES,
    .sample_rate = SPK_AUDIO_RATE,
};


static DEFINE_SPINLOCK(pc_spk_lock);

void pc_spk_data_isr_cb(void *buf, u32 len)
{
    struct pc_spk_file_hdl *hdl = pc_spk;
    struct stream_frame *frame = NULL;

    int wlen = 0;

    if (!hdl) {
        if (pc_spk_player_runing() == 0) {
            //打开播放器
            pcspk_open_player_by_taskq();
        }
        return;
    }
    struct stream_node  *source_node = hdl->source_node;
    u32 pcrx_isr_timestamp = audio_jiffies_usec();
    if (hdl->start == 0) {
        if (!hdl->cache_buf) {
            //申请cbuffer
            hdl->cache_buf = malloc(PC_SPK_CACHE_BUF_LEN);
            if (hdl->cache_buf) {
                cbuf_init(&hdl->spk_cache_cbuffer, hdl->cache_buf, PC_SPK_CACHE_BUF_LEN);
            }
        } else {
            /* wlen = cbuf_write(&hdl->spk_cache_cbuffer, buf, len); */
            /* if (wlen != len) { */
            /* putchar('W'); */
            /* } */
        }
        return;
    }

#if PC_SPK_ONLINE_DET_EN
    hdl->irq_cnt++;
#endif

    //1ms 起一次中断，一次长度192, 中断太快,需缓存
    wlen = cbuf_write(&hdl->spk_cache_cbuffer, buf, len);
    if (wlen != len) {
        putchar('W');
    }
    u32 cache_len = cbuf_get_data_len(&hdl->spk_cache_cbuffer);
    if (cache_len >= len * 5) {
        frame = source_plug_get_output_frame(source_node, cache_len);
        if (!frame) {
            return;
        }
        frame->len    = cache_len;
        frame->flags  = FRAME_FLAG_SYS_TIMESTAMP_ENABLE;
        frame->timestamp = pcrx_isr_timestamp;
        cbuf_read(&hdl->spk_cache_cbuffer, frame->data, frame->len);
        source_plug_put_output_frame(source_node, frame);
        hdl->data_run = 1;
    }
}

/* 定时器检测 pcspk 在线 */
static void pcspk_det_timer_cb(void *priv)
{
    struct pc_spk_file_hdl *hdl = (struct pc_spk_file_hdl *)priv;
    if (hdl) {
        if (hdl->start) {
            if (hdl->irq_cnt) {
                hdl->irq_cnt = 0;
            } else {
                if (hdl->data_run) {
                    //已经往后面推数据突然中断没有起的情况
                    hdl->data_run = 0;
                    r_printf(">>>>>>> PCSPK LOST CONNECT <<<<<<<");
                    pcspk_close_player_by_taskq();
                }
            }
            s16 cur_vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
            u16 l_vol = 0, r_vol = 0;
            uac_speaker_stream_get_volume(&l_vol, &r_vol);
            if (cur_vol != (l_vol + r_vol) / 2) {
                app_audio_set_volume(APP_AUDIO_STATE_MUSIC, (r_vol + l_vol) / 2, 1);
            }
        }
    }
}

/*
 * 申请 pc_spk_file 结构体内存空间
 */
static void *pc_spk_file_init(void *source_node, struct stream_node *node)
{
    struct pc_spk_file_hdl *hdl = NULL;
    if (pc_spk != NULL) {
        hdl = pc_spk;
    } else {
        hdl = zalloc(sizeof(*hdl));
    }
    if (!hdl) {
        r_printf("%s, %d, alloc memory failed!\n", __func__, __LINE__);
        return NULL;
    }

    hdl->source_node = source_node;
    hdl->node = node;
    pc_spk = hdl;
    return hdl;
}

static int pc_spk_ioc_get_fmt(struct pc_spk_file_hdl *hdl, struct stream_fmt *fmt)
{
    fmt->coding_type = AUDIO_CODING_PCM;
    if (pc_spk_fmt.channel == 2) {
        fmt->channel_mode   = AUDIO_CH_LR;
    } else {
        fmt->channel_mode   = AUDIO_CH_L;
    }
    fmt->sample_rate    = pc_spk_fmt.sample_rate;
    fmt->bit_wide = (pc_spk_fmt.bit == 24) ? 1 : 0;
    fmt->pcm_24bit_type = (pc_spk_fmt.bit == 24) ? PCM_24BIT_DATA_3BYTE : PCM_24BIT_DATA_4BYTE;
    y_printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> fmt->bit_wide : %d\n", fmt->bit_wide);
    return 0;
}

static int pc_spk_ioc_set_fmt(struct pc_spk_file_hdl *hdl, struct stream_fmt *fmt)
{
    return 0;
}

//打开pcspk 在线检测定时器
static void pcspk_open_det_timer(void)
{
#if PC_SPK_ONLINE_DET_EN
    struct pc_spk_file_hdl *hdl = pc_spk;
    //申请定时器
    if (hdl) {
        if (!hdl->det_timer_id) {
            hdl->det_timer_id = usr_timer_add(hdl, pcspk_det_timer_cb, PC_SPK_ONLINE_DET_TIME, 0);
        }
    }
#endif
}

//关闭 pcspk 在线检测定时器
static void pcspk_close_det_timer(void)
{
#if PC_SPK_ONLINE_DET_EN
    struct pc_spk_file_hdl *hdl = pc_spk;
    //申请定时器
    if (hdl) {
        if (hdl->det_timer_id) {
            usr_timer_del(hdl->det_timer_id);
            hdl->det_timer_id = 0;
        }
    }
#endif
}

#if 1
static int pc_spk_ioctl(void *_hdl, int cmd, int arg)
{
    u32 i = 0;
    int ret = 0;
    struct pc_spk_file_hdl *hdl = (struct pc_spk_file_hdl *)_hdl;
    switch (cmd) {
    case NODE_IOC_GET_FMT:
        pc_spk_ioc_get_fmt(hdl, (struct stream_fmt *)arg);
        break;
    case NODE_IOC_SET_FMT:
        ret = pc_spk_ioc_set_fmt(hdl, (struct stream_fmt *)arg);
        break;
    case NODE_IOC_SET_PRIV_FMT:
        ret = pc_spk_ioc_set_fmt(hdl, (struct stream_fmt *)arg);
        break;

    case NODE_IOC_START:
        if (hdl->start == 0) {
            if (!hdl->cache_buf) {
                //申请cbuffer
                hdl->cache_buf = malloc(PC_SPK_CACHE_BUF_LEN);
                if (hdl->cache_buf) {
                    cbuf_init(&hdl->spk_cache_cbuffer, hdl->cache_buf, PC_SPK_CACHE_BUF_LEN);
                    //打开检测 pcspk 是否起中断的定时器
                    pcspk_open_det_timer();
                    hdl->data_run = 0;
                    hdl->start = 1;
                }
            } else {
                pcspk_open_det_timer();
                hdl->data_run = 0;
                hdl->start = 1;
            }
        }
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        if (hdl->start) {
            hdl->start = 0;
        }
        break;
    }
    return ret;
}
#endif


static void pc_spk_release(void *_hdl)
{
    struct pc_spk_file_hdl *hdl = (struct pc_spk_file_hdl *)_hdl;

    spin_lock(&pc_spk_lock);
    if (!hdl) {
        hdl = pc_spk;
        if (!hdl) {
            spin_unlock(&pc_spk_lock);
            return;
        }
    }

    pcspk_close_det_timer();
    free(hdl->cache_buf);
    hdl->cache_buf = NULL;
    free(hdl);
    hdl = NULL;
    pc_spk = NULL;
    spin_unlock(&pc_spk_lock);
}

u8 is_pc_spk_file_start(void)
{
    struct pc_spk_file_hdl *hdl = pc_spk;
    if (!hdl) {
        return 0;
    }
    return (hdl->start);
}

// 设置pc mic 的数据格式，传入0不设置
void pc_spk_set_fmt(u8 channel, u8 bit, u32 sample_rate)
{
    y_printf("----------- Call %s, bit:%d, sr:%d\n", __func__, bit, sample_rate);
    pc_spk_fmt.init = 1;
    if (channel != 0) {
        pc_spk_fmt.channel = channel;
    }
    if (bit != 0) {
        pc_spk_fmt.bit = bit;
    }
    if (sample_rate != 0) {
        pc_spk_fmt.sample_rate = sample_rate;
    }
}

u32 pc_spk_get_fmt_sample_rate(void)
{
    return pc_spk_fmt.sample_rate;
}


REGISTER_SOURCE_NODE_PLUG(pc_spk_file_plug) = {
    .uuid       = NODE_UUID_PC_SPK,
    .init       = pc_spk_file_init,
    .ioctl      = pc_spk_ioctl,
    .release    = pc_spk_release,
};

#endif


