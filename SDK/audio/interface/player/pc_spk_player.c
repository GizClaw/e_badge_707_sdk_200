#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".pc_spk_player.data.bss")
#pragma data_seg(".pc_spk_player.data")
#pragma const_seg(".pc_spk_player.text.const")
#pragma code_seg(".pc_spk_player.text")
#endif
#include "jlstream.h"
#include "system/timer.h"
#include "system/includes.h"
#include "pc_spk_player.h"
#include "pc_spk_file.h"
#include "app_config.h"
#include "app_main.h"
#include "effects/audio_pitchspeed.h"
#include "effect/effects_default_param.h"
#include "audio_config.h"
#include "scene_switch.h"
#include "uac_stream.h"

#if TCFG_USB_SLAVE_AUDIO_SPK_ENABLE

struct pc_spk_player {
    struct jlstream *stream;
    u8 open_flag;
    s8 pc_spk_pitch_mode;
};
static struct pc_spk_player *g_pc_spk_player = NULL;

// 防止通过系统任务重复调用关闭、打开player
static u8 player_wait_close_flag = 0;
static u8 player_wait_open_flag = 0;
static u8 player_wait_restart_flag = 0;

static void pc_spk_player_callback(void *private_data, int event)
{
    struct pc_spk_player *player = g_pc_spk_player;
    struct jlstream *stream = (struct jlstream *)private_data;

    switch (event) {
    case STREAM_EVENT_START:
        u16 l_vol = 0, r_vol = 0;
        uac_speaker_stream_get_volume(&l_vol, &r_vol);
        app_audio_set_volume(APP_AUDIO_STATE_MUSIC, (r_vol + l_vol) / 2, 1);
        break;
    }
}

/*
 * @description: 打开 pc spk 数据流
 * @return：0 - 成功。其它值失败
 * @node:
 */
int pc_spk_player_open(void)
{
    int err = 0;
    struct pc_spk_player *player = NULL;;

    player_wait_open_flag = 0;
    if (g_pc_spk_player) {
        return 0;
    }

    u16 uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"pc_spk");
    if (uuid == 0) {
        return -EFAULT;
    }

    player = zalloc(sizeof(*player));
    if (!player) {
        return -ENOMEM;
    }
    player->pc_spk_pitch_mode = PITCH_0;
    player->stream = jlstream_pipeline_parse(uuid, NODE_UUID_PC_SPK);

    if (!player->stream) {
        err = -ENOMEM;
        goto __exit0;
    }
    jlstream_node_ioctl(player->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PRIV_FMT, 192);
    jlstream_set_callback(player->stream, player->stream, pc_spk_player_callback);
    jlstream_set_scene(player->stream, STREAM_SCENE_PC_SPK);
    err = jlstream_start(player->stream);
    if (err) {
        goto __exit1;
    }

    player->open_flag = 1;
    g_pc_spk_player = player;
    return 0;

__exit1:
    jlstream_release(player->stream);
__exit0:
    free(player);
    return err;
}

// 返回1说明player 在运行
bool pc_spk_player_runing()
{
    return g_pc_spk_player != NULL;
}

/*
 * @description: 关闭spk 数据流
 * @return：
 * @node:
 */
void pc_spk_player_close(void)
{
    struct pc_spk_player *player = g_pc_spk_player;

    player_wait_close_flag = 0;
    if (!player) {
        return;
    }

    player->open_flag = 0;

    if (is_pc_spk_file_start()) {
        jlstream_stop(player->stream, 0);
    }
    jlstream_release(player->stream);

    free(player);
    player = NULL;
    g_pc_spk_player = NULL;
    jlstream_event_notify(STREAM_EVENT_CLOSE_PLAYER, (int)"pc_spk");
}

static void pc_spk_player_restert(void)
{
    struct pc_spk_player *player = g_pc_spk_player;

    player_wait_restart_flag = 0;
    if (player && player->open_flag) {
        pc_spk_player_close();
        pc_spk_player_open();
    }
}


int pcspk_close_player_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    if (player_wait_close_flag == 0) {
        msg[0] = (int)pc_spk_player_close;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        player_wait_close_flag = 1;
    }
    return ret;
}


int pcspk_open_player_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    if (player_wait_open_flag == 0) {
        msg[0] = (int)pc_spk_player_open;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        player_wait_open_flag = 1;
    }
    return ret;
}

int pcspk_restart_player_by_taskq(void)
{
    int msg[2];
    int ret = 0;
    if (player_wait_restart_flag == 0) {
        msg[0] = (int)pc_spk_player_restert;
        msg[1] = 0;
        ret = os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
        player_wait_restart_flag = 1;
    }
    return ret;
}

#else

bool pc_spk_player_runing()
{
    return 0;
}


#endif


