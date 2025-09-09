#include "app_config.h"
#include "ui/ui_api.h"
#include "system/timer.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_CS_VOLUE_CONTROLS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_volue_control.data.bss")
#pragma data_seg(".ui_cs_volue_control.data")
#pragma const_seg(".ui_cs_volue_control.text.const")
#pragma code_seg(".ui_cs_volue_control.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_VOLUME_ENABLE) && TCFG_UI_VOLUME_ENABLE

#define STYLE_NAME  JL

typedef enum {
    ICON_STATUS_DISCONNECT = 0,
    ICON_STATUS_CONNECT,
    ICON_STATUS_PRESSING,
} ICON_STATUS;

REGISTER_UI_STYLE(STYLE_NAME)


static int volume_status_update(const char *type, uint32_t argc)
{

    struct unumber vol_num = {0};
    vol_num.number[0] = sbox_volume_get();
    if (vol_num.number[0] >= 16) {
        vol_num.number[0] = 16;
    } else if (vol_num.number[0] <= 0) {
        vol_num.number[0] = 0;
    }
    ui_number_update_by_id(VOLUME_DISPLAY_NUM, &vol_num);
    return 0;
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "volume", volume_status_update },
    { NULL, NULL } // 必须以此结尾
};

static int VOLUME_DISPLAY_NUM_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        struct unumber vol_num = {0};
        struct ui_number *number = (struct ui_number *)elm;

        vol_num.type = TYPE_NUM;
        if (strcmp(number->source, "vol_num") == 0) {
            vol_num.number[0] = sbox_volume_get();
        }

        if (vol_num.number[0] >= 16) {
            vol_num.number[0] = 16;
        } else if (vol_num.number[0] <= 0) {
            vol_num.number[0] = 0;
        }
        ui_number_update(number, &vol_num);
        ui_register_msg_handler(ID_WINDOW_VOLUME, ui_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(VOLUME_DISPLAY_NUM)
.onchange = VOLUME_DISPLAY_NUM_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int cs_img_volume_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        switch (pic->elm.id) {
        case VOLUME_ADD_PIC:
            if (sbox_ble_connect_flag_get()) {
                ui_pic_set_image_index(pic, ICON_STATUS_CONNECT);
            } else {
                ui_pic_set_image_index(pic, ICON_STATUS_DISCONNECT);
            }
            break;

        case VOLUME_SUB_PIC:
            if (sbox_ble_connect_flag_get()) {
                ui_pic_set_image_index(pic, ICON_STATUS_CONNECT);
            } else {
                ui_pic_set_image_index(pic, ICON_STATUS_DISCONNECT);
            }
            break;

        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        break;
    }
    return false;
}

static int cs_img_volume_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }

        if (!sbox_ble_connect_flag_get()) {
            return false;
        }

        int volue = 0;
        switch (pic->elm.id) {
        case VOLUME_ADD_PIC:
            volue = sbox_volume_get();
            if (volue >= 16) {
                break;
            }
            custom_client_send_volume_up();
            break;

        case VOLUME_SUB_PIC:
            volue = sbox_volume_get();
            if (volue <= 0) {
                break;
            }
            custom_client_send_volume_down();
            break;

        default:
            break;
        }
        return true;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(VOLUME_ADD_PIC)
.onchange = cs_img_volume_onchange,
 .onkey = NULL,
  .ontouch = cs_img_volume_ontouch,
};

REGISTER_UI_EVENT_HANDLER(VOLUME_SUB_PIC)
.onchange = cs_img_volume_onchange,
 .onkey = NULL,
  .ontouch = cs_img_volume_ontouch,
};


void ui_volume_window_bt_status_update(void)
{
    struct ui_pic *pic = NULL;

    pic = (struct ui_pic *)ui_core_get_element_by_id(VOLUME_SUB_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }

    pic = (struct ui_pic *)ui_core_get_element_by_id(VOLUME_ADD_PIC);
    if (pic) {
        ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
    }
}


#endif //TCFG_UI_VOLUME_ENABLE
#endif //CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE

