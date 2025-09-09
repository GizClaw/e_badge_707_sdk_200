#include "app_config.h"
#include "system/includes.h"
#include "app_task.h"
#include "ascii.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "dev_manager/dev_manager.h"
#include "fs/fs_file_name.h"
#include "multi_protocol_main.h"
#include "rcsp_cmd_user.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_UI_ENABLE && ((defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) ||(defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE)))

void *dial_zalloc(int size);
void dial_free(void *buf);
void virfat_flash_get_dirinfo(void *file_buf, u32 *file_num);
// 内存管理接口，由外部注册进来
static void *(*dial_buf_malloc)(int size);
static void (*dial_buf_free)(void *p);
static int watch_mem_bgp_related();

void  dial_manage_memory(void *(*malloc)(int size), void (*free)(void *p))
{
    dial_buf_malloc = malloc;
    dial_buf_free = free;
}
void *dial_zalloc(int size)
{
    void *p;
    if (dial_buf_malloc) {
        p = dial_buf_malloc(size);
    } else {
        p = malloc(size);
    }
    memset(p, 0, size);
    ASSERT(p, "dial malloc buffer faild, size: %d\n", size);
    return p;
}
void dial_free(void *buf)
{
    if (dial_buf_free) {
        dial_buf_free(buf);
    } else {
        free(buf);
    }
}
/********************************************************
*		表盘管理
********************************************************/
#define WATCH_ITEMS_LIMIT      	10
#define BGP_ITEMS_LIMIT        	10

#define WATCH_ITEMS_MIN_LIMIT   2   /*表盘最少数量*/

#define WATCH_MEM_BGP       0x55aa66bb

struct dial_manage {
    u8 dial_cur_style;
    u8 standby_cur_style;
    u8 update_over;
    u8 dial_items;
    u8 bgp_items;

    u8 watch_need_load;
    u8 dial_remap_bgp[WATCH_ITEMS_LIMIT];
    char *dial_path[WATCH_ITEMS_LIMIT];
    char *bgp_path[BGP_ITEMS_LIMIT];            /*表盘背景*/
    char *bgp_related_path[BGP_ITEMS_LIMIT];    /*按表盘顺序对应的表盘背景*/

    /*表盘对应背景图记忆*/
    u32 wmem_last;
    u32 wmem_area_num;
    void *wmem_file;
    u8 wmem_init_flag;
};

struct dial_store {
    u8 dial_cur_style;
    u8 standby_cur_style;
};
const char *WATCH_VERSION_LIST = {
    "W003"
};

static struct dial_manage dial_manage_info;
#define __this  (&dial_manage_info)




/********************************************************
*			func
********************************************************/
void watch_set_need_reload(u8 need)
{
    __this->watch_need_load = !!need;
}
u32 watch_bgp_get_nums()
{
    return __this->bgp_items;
}

char *watch_bgp_get_item(u8 sel_item)
{
    if (sel_item >= __this->bgp_items) {
        return NULL;
    }
    return __this->bgp_path[sel_item];
}

char *watch_bgp_get_item_without_path(s8 sel_item)
{
    if (sel_item < 0) {
        return NULL;
    }
    if (sel_item >= __this->bgp_items) {
        return NULL;
    }
    log_info("%s %s ", __func__, __this->bgp_path[sel_item]);
    return __this->bgp_path[sel_item] + strlen(RES_PATH) ;
}

/**
 * @brief 添加表盘路径
 *
 * @param bgp     A pointer to a string.(e.g. "bgp_w000")
 * @return char*  A pointer to a string.(e.g. "storage/virfat_flash/C/bgp_w000")
 */
char *watch_bgp_add(char *bgp)
{
    log_info("<%s>", __func__);
    if (!bgp) {
        log_error("<%s> bgp is null !!!", __func__);
        return NULL;
    }
    char *root_path = RES_PATH;
    char *new_bgp_item = NULL;
    u32 len;
    u32 i;

    log_info("%s %d %s", __func__, __LINE__, bgp);
    if ((__this->bgp_items + 1) >= BGP_ITEMS_LIMIT) {
        log_error("\n\n\nwatch_bgp items %d overflow \n\n\n\n", __this->bgp_items);
        return NULL;
    }

    len = strlen(bgp) + strlen(root_path) + 1;
    new_bgp_item = dial_zalloc(len);
    if (new_bgp_item == NULL) {
        log_error("\n\n\nwatch_bgp items malloc err %d\n\n\n\n", len);
        return NULL;
    }

    ASCII_ToLower(bgp, strlen(bgp));
    strcpy(new_bgp_item, root_path);
    strcpy(&new_bgp_item[strlen(root_path)], bgp);
    new_bgp_item[len - 1] = '\0';

    log_info("%s %d %s", __func__, __LINE__, new_bgp_item);

    /*文件不存在情况*/
    if (!fopen(new_bgp_item, "r")) {
        log_error("new_bgp_item does not exist", new_bgp_item);
        return NULL;
    }

    //如果已经存在这个背景图片路径，则直接返回对应的地址
    for (i = 0; i < __this->bgp_items; i++) {
        /* log_info("num %s\n", watch_bgp[i]); */
        log_info("%s i:%d new:%s bgp_path:%s \n", __func__, i, new_bgp_item, __this->bgp_path[i]);
        if ((strncmp(new_bgp_item, __this->bgp_path[i], strlen(new_bgp_item)) == 0) &&
            (strlen(__this->bgp_path[i]) == strlen(new_bgp_item))) {
            dial_free(new_bgp_item);
            new_bgp_item = __this->bgp_path[i];
            log_info("already has %s\n", new_bgp_item);
            return new_bgp_item;
        }
    }

    log_info("%s %d %s", __func__, __LINE__, new_bgp_item);
    __this->bgp_path[__this->bgp_items] = new_bgp_item;
    __this->bgp_items++;

    log_info("add new_bgp_item succ %d, %s\n", __this->bgp_items, new_bgp_item);

    return new_bgp_item;
}

/**
 * @brief 删除表盘路径
 *
 * @param bgp   A pointer to a string.(e.g. "bgp_w000")
 * @return int  0:success -1:fail
 */
int watch_bgp_del(char *bgp)
{
    u32 i;
    char watch_bgp_item[64];
    u32 cur_items = __this->bgp_items;
    char *root_path = RES_PATH;
    char *bgp_item = NULL;

    log_info("<%s>", __func__);
    ASSERT(((strlen(bgp) + strlen(root_path) + 1) < sizeof(watch_bgp_item)), "bgp err name0 %s\n", bgp);

    ASCII_ToLower(bgp, strlen(bgp));
    strcpy(watch_bgp_item, root_path);
    /* strcpy(&watch_bgp_item[strlen(root_path)], &bgp[1]); */
    strcpy(&watch_bgp_item[strlen(root_path)], bgp);
    watch_bgp_item[strlen(bgp) + strlen(root_path)] = '\0';
    log_info("watch_bgp_item %s\n", watch_bgp_item);

    for (i = 0; i < cur_items; i++) {
        if (strncmp(watch_bgp_item, __this->bgp_path[i], strlen(watch_bgp_item)) == 0) {
            bgp_item = __this->bgp_path[i];
            __this->bgp_path[i] = NULL;
            dial_free(bgp_item);
            __this->bgp_items--;
            break;
        }
    }

    if (bgp_item == NULL) {
        log_info("can not find bgp_item %s\n", watch_bgp_item);
        return -1;
    }

    for (; i < cur_items; i++) {
        if (__this->bgp_path[i + 1] != NULL) {
            __this->bgp_path[i] = __this->bgp_path[i + 1];
        } else {
            __this->bgp_path[i] = NULL;
            break;
        }
    }

    //del related item
    cur_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    for (i = 0; i < cur_items; i++) {
        if (bgp_item == __this->bgp_related_path[i]) {
            __this->bgp_related_path[i] = NULL;
        }
    }

    for (i = 0; i < cur_items; i++) {
        log_info("cur related items %d, %s\n", i, __this->bgp_related_path[i]);
    }
    for (i = 0; i < __this->bgp_items; i++) {
        log_info("cur bgp items %d, %s\n", i, __this->bgp_path[i]);
    }
    return 0;
}

/**
 * @brief 设置表盘对应的背景图路径
          1.如果对应表盘没有对应的背景图，则增加
          2.如果对应表盘有对应背景图，则替换
          e.g.
          watch_bgp_set_related("bgp_w000", 0, 1); 设置表盘背景图为"bgp_w000"
          watch_bgp_set_related(NULL, 0, 1); 不设置表盘背景图
 *
 * @param bgp       A pointer to a string.(e.g. "bgp_w000")
 * @param cur_watch 表盘序号
 * @param del       1:如果被替换的背景图不再关联任何表盘，则将它从bgp_path[]中删除 0:不会删除
 * @return int      0:success -1:fail
 */
int watch_bgp_set_related(char *bgp, u8 cur_watch, u8 del)
{
    u32 i;
    u32 cur_items;
    char *root_path = RES_PATH;
    char *bgp_item = NULL;
    char *new_bgp_item = NULL;
    u32 total_relate_items;
    char old_bgp[16] = {0};
    u32 len;

    log_info("<%s> bgp:%s cur_watch:%d del:%d", __func__, bgp, cur_watch, del);
    total_relate_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    if (cur_watch >= total_relate_items) {
        log_error("<%s> cur_watch:%d is error", __func__, cur_watch);
        return -1;
    }

    //提取旧的背景图片名字
    if (__this->bgp_related_path[cur_watch]) {
        bgp_item = __this->bgp_related_path[cur_watch];
        len = strlen(bgp_item) - strlen(root_path) + 1;
        if (len >= sizeof(old_bgp)) {
            return -1;
        }
        memcpy(old_bgp, &bgp_item[strlen(root_path)], len);
        log_info("cur_watch:%d old bpg %s\n", cur_watch, old_bgp);
    }

    if (bgp != NULL) {
        new_bgp_item = watch_bgp_add(bgp);
        if (new_bgp_item == NULL) {
            log_error("add bgp item err %s\n", bgp);
            return -1;
        }
    }

    //如果没有背景图片
    if (__this->bgp_related_path[cur_watch] == NULL) {
        __this->bgp_related_path[cur_watch] = new_bgp_item;
    } else {
        bgp_item = __this->bgp_related_path[cur_watch];
        __this->bgp_related_path[cur_watch] = new_bgp_item;
        cur_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
        for (i = 0; i < cur_items; i++) {
            if (bgp_item == __this->bgp_related_path[i]) {
                break;
            }
        }
        if ((i == cur_items) && del) { //被替换的这个背景图片已经不关联任何表盘
            watch_bgp_del(old_bgp);
        }
    }

    watch_mem_bgp_related();

    return 0;
}

/**
 * @brief 获取当前表盘对应的背景图
 *
 * @param cur_watch 表盘序号
 * @return char* A pointer to a string.(e.g. "/bgp_w000")
 */
char *watch_bgp_get_related(u8 cur_watch)
{
    u32 total_relate_items;

    log_info("<%s> cur_watch:%d", __func__, cur_watch);
    total_relate_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    if (cur_watch >= total_relate_items) {
        log_error("\n\n\nwatch_bgp_related items %d overflow \n\n\n\n", cur_watch);
        return NULL;
    }
    if (!__this->bgp_related_path[cur_watch]) {
        return NULL;
    }

    log_info("%s cur_items:%d total_relate_items:%d path:%s",
             __func__, cur_watch, total_relate_items, __this->bgp_related_path[cur_watch]);
    ASSERT(strlen(__this->bgp_related_path[cur_watch]) > strlen(RES_PATH));
    return __this->bgp_related_path[cur_watch] + strlen(RES_PATH) - 1;
}
/**
 * @brief 获取当前表盘对应的背景图d
 *
 * @param cur_watch 表盘序号
 * @return char* A pointer to a string.(e.g. "storage/virfat/C/bgp_w000")
 */
char *watch_bgp_get_related_path(u8 cur_watch)
{
#if (defined TCFG_JL_WALLPAPER_ENABLE && TCFG_JL_WALLPAPER_ENABLE)
    extern char* jl_wallpaper_get_path();
    return jl_wallpaper_get_path();
#else
    u32 total_relate_items;

    log_info("<%s>", __func__);
    total_relate_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    if (cur_watch >= total_relate_items) {
        log_error("\n\n\nwatch_bgp_related items %d overflow \n\n\n\n", cur_watch);
        return NULL;
    }
    return __this->bgp_related_path[cur_watch];
#endif
}
char *watch_bgp_get_name(int item)
{
    static char name[16];
    char *path = __this->bgp_path[item];
    log_info("<%s> path:%s", __func__, path);
    for (int i = strlen(path) - 1; i > 0 ; i--) {
        if (path[i] == '/') {
            i++;//map '/'
            if (strlen(&path[i]) < 16) {
                memset(name, 0, 16);
                memcpy(name, &path[i], strlen(&path[i]));
                ASCII_ToUpper(name, strlen(name));
                return name;
            } else {
                ASSERT(0);
                return NULL;
            }
        }
    }
    return NULL;
}
int watch_bgp_get_index(char *bgp_name)
{
    if (bgp_name == NULL) {
        return -1;
    }
    for (int i = 0; i < BGP_ITEMS_LIMIT; i++) {
        log_info("%s i:%d path:%s bgp:%s", __func__, i, __this->bgp_path[i] + strlen(RES_PATH) - 1, bgp_name);
        if (__this->bgp_path[i]) {
            char *check_name = __this->bgp_path[i] + strlen(RES_PATH) - 1;
            if (!strncmp(check_name, bgp_name, strlen(bgp_name)) &&
                (strlen(check_name) == strlen(bgp_name))) {
                return i;
            }
        }
    }
    return -1;
}
/**
 * @brief 删除表盘背景图并记忆
 *
 * @param bgp A pointer to a string.(e.g. "bgp_w000")
 */
int watch_bgp_related_del_all(char *bgp)
{
    log_info("<%s>", __func__);
    watch_bgp_del(bgp);
    watch_mem_bgp_related();
    return 0;
}

int watch_get_style_by_name(char *name)
{
    u32 i;
    for (i = 0; i < __this->dial_items; i++) {
        if (strncmp(name, __this->dial_path[i], strlen(name)) == 0) {
            return i;
        }
    }
    return -1;
}


int watch_mem_new(u32 area)
{
    log_info("<%s>", __func__);
    __this->wmem_last = area;
    __this->wmem_area_num++;
    return 0;
}

void *watch_mem_open()
{
    log_info("<%s>", __func__);
    if (__this->wmem_file == NULL) {
        __this->wmem_file = fopen(RES_PATH"wmem.bin", "w+");
    }
    return __this->wmem_file;
}

void watch_mem_close()
{
    log_info("<%s>", __func__);
    if (__this->wmem_file) {
        fclose(__this->wmem_file);
        __this->wmem_file = NULL;
    }
}

int watch_mem_write(u32 offset, u32 len, u8 *buf, u32 area)
{
    u32 ret;
    u8 tmp_buf[8];
    u32 area_offset = 0;
    u32 area_len = 0;
    u32 find_tag = 0;

    log_info("<%s>", __func__);
    if (__this->wmem_file == NULL) {
        return -1;
    }

    if ((flen(__this->wmem_file) == 0) || (__this->wmem_area_num <= 1)) {
        area_offset = 0;
    } else {
        do {
            fseek_fast(__this->wmem_file, area_offset, SEEK_SET);
            ret = fread_fast(tmp_buf, 8, 1, __this->wmem_file);
            if (ret != 8) {
                log_error("wmem find tag err end\n");
                return -1;
            }
            memcpy(&ret, tmp_buf + 4, 4); //flag
            if (ret == area) {
                memcpy(&area_len, tmp_buf, 4);//len
                find_tag = 1;
                break;
            }
            memcpy(&ret, tmp_buf, 4);//len
            area_offset += ret;
        } while (find_tag == 0);
    }

    fseek_fast(__this->wmem_file, area_offset + offset, SEEK_SET);
    if (area == __this->wmem_last) {
        ret = fwrite(buf, len, 1, __this->wmem_file);

        if (ret != len) {
            return -1;
        }
    } else {
        if ((offset + len) <= area_len) {
            ret = fwrite(buf, len, 1, __this->wmem_file);
            if (ret != len) {
                return -1;
            }
        } else {
            //要将这个区域的内容先搬迁到文件最后，使这个区域成为最后的区域
            //再写数据

        }
    }

    return 0;
}

int watch_mem_read(u32 offset, u32 len, u8 *buf, u32 area)
{
    u32 area_offset = 0;
    u32 area_len = 0;
    u32 find_tag = 0;
    u32 ret;
    u8 tmp_buf[8];

    log_info("<%s>", __func__);
    if (__this->wmem_file == NULL) {
        return -1;
    }

    if (flen(__this->wmem_file) == 0) {
        return 0;
    }


    if (__this->wmem_area_num <= 1) {
        area_offset = 0;
    } else {
        do {
            fseek_fast(__this->wmem_file, area_offset, SEEK_SET);
            ret = fread_fast(tmp_buf, 8, 1, __this->wmem_file);
            if (ret != 8) {
                log_error("wmem find tag err end\n");
                return -1;
            }
            memcpy(&ret, tmp_buf + 4, 4); //flag
            if (ret == area) {
                memcpy(&area_len, tmp_buf, 4);//len
                find_tag = 1;
                break;
            }
            memcpy(&ret, tmp_buf, 4);//len
            area_offset += ret;
        } while (find_tag == 0);

        if ((offset + len) > area_len) {
            return -1;
        }
    }

    fseek_fast(__this->wmem_file, area_offset + offset, SEEK_SET);
    ret = fread_fast(buf, len, 1, __this->wmem_file);
    if (ret != len) {
        log_error("wmem read err %d\n", ret);
        return -1;
    }

    return 0;
}

/**
 * @brief 记忆当前表盘对应的背景图
 *
 * @return int 0:success -1:fail
 */
static int watch_mem_bgp_related()
{
    int ret = 0;
    u8 *related_buf;
    u32 total_relate_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    u32 len;
    char *related_item;
    u32 area = WATCH_MEM_BGP;
    u32 i;

    log_info("<%s>", __func__);
    if (__this->bgp_items == 0) {
        return -1;
    }

    len = 64 * (total_relate_items + 1);
    related_buf = dial_zalloc(len);
    if (related_buf == NULL) {
        return -1;
    }

    if (watch_mem_open() == NULL) {
        dial_free(related_buf);
        return -1;
    }

    memcpy(related_buf, &len, 4);
    memcpy(related_buf + 4, &area, 4);
    for (i = 0; i < total_relate_items; i++) {
        related_item = __this->bgp_related_path[i];
        if (related_item) {
            log_info("related item : %s, %d, %d\n", related_item, (int)(strlen(related_item) + 1), i);
            memcpy(&related_buf[64 + 64 * i], related_item, strlen(related_item) + 1);
        }
    }

    ret = watch_mem_write(0, len, related_buf, area);
    if (ret != 0) {
        log_info("watch mem werr %x\n", ret);
    } else {
        log_info("watch mem succ\n");
    }

    watch_mem_close();

    dial_free(related_buf);

    return ret;
}

/**
 * @brief 初始化表盘背景图路径
 *
 * @return int 0:success -1:fail
 */
static int watch_bgp_related_init()
{
    int ret = 0;
    u8 *related_buf;
    u32 total_relate_items = sizeof(__this->bgp_related_path) / sizeof(__this->bgp_related_path[0]);
    u32 len;
    char *related_item;
    u32 area = WATCH_MEM_BGP;
    u32 i, j;

    log_info("<%s>", __func__);
    if (__this->wmem_init_flag == 0) {
        watch_mem_new(area);
        __this->wmem_init_flag = 1;
    }

    if (__this->bgp_items == 0) {
        log_info("<%s> line:%d", __func__, __LINE__);
        return -1;
    }

    len = 64 * (total_relate_items + 1);
    related_buf = dial_zalloc(len);
    if (related_buf == NULL) {
        log_info("<%s> line:%d", __func__, __LINE__);
        return -1;
    }

    if (watch_mem_open() == NULL) {
        dial_free(related_buf);
        log_info("<%s> line:%d", __func__, __LINE__);
        return -1;
    }

    ret = watch_mem_read(0, len, related_buf, area);
    if (ret != 0) {
        dial_free(related_buf);
        log_info("<%s> line:%d", __func__, __LINE__);
        return -1;
    }

    for (i = 0; i < total_relate_items; i++) {
        __this->bgp_related_path[i] = NULL;
        related_item = (char *)&related_buf[64 + 64 * i];
        if (related_item[0]) {
            log_info("related item : %s, %d, %d\n", related_item, (int)strlen(related_item), i);
            for (j = 0; j < __this->bgp_items; j++) {
                if (strncmp(related_item, __this->bgp_path[j], strlen(__this->bgp_path[j])) == 0) {
                    __this->bgp_related_path[i] = __this->bgp_path[j];
                    log_info("find bgp related %d\n", j);
                    break;
                }
            }
        }
    }

#ifdef CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
    //默认设置第一个背景图片对应为表盘样式背景图片
    if (ret <= 0) {
        char *bgp = watch_bgp_get_item_without_path(0);
        log_info("%s set bgp:%s\n", __func__, index, bgp);
        watch_bgp_set_related(bgp,	watch_get_style(),	0);
    }
#endif

    watch_mem_close();

    dial_free(related_buf);

    return 0;
}

char *watch_get_background()
{
    return NULL;
}

int watch_set_background(char *bg_pic)
{
    return 0;
}

void watch_update_finish()
{
}

int watch_get_update_status()
{
    return 0;
}

static int watch_get_msg_by_path(char *key, char *data, u32 data_len, u32 offset)
{
    int ret = 0;
    u32 i = 0;
    u32 tmp_strlen = 0;
    u8 *tmp_data = NULL;
    UI_RESFILE *file = res_fopen(data, "r");
    if (NULL == file) {
        ret = -1;
        goto _end;
    }
    // 获取长度
    tmp_strlen = res_flen(file);
    // 用来装路径名字
    tmp_data = dial_zalloc(strlen(data) + 1);
    if (!tmp_data) {
        ret = -1;
        goto _end;
    }
    memcpy(tmp_data, data, strlen(data) + 1);
    memset(data, 0, data_len);
    // 读出数据
    res_fseek(file, offset, SEEK_SET);
    res_fread(file, data, data_len);

    // 关闭数据
    if (file) {
        res_fclose(file);
        file = NULL;
    }

    // 判断表盘的json文件一开始是否符合规范，不符合直接返回(该判断针对读出来是全f的数据)
    if ((0 == offset) && (0x7B != data[0] || 0x0D != data[1] || 0x0A != data[2])) {
        ret = -1;
        goto _end;
    }

    char *tver = NULL;
    for (i = 0; i < data_len; i++) {
        // 换行
        if (0xd == data[i] && 0x0a == data[i + 1]) {
            offset += i + 1 + 1;
            break;
        }
        // 文件结束
        if (0 == data[i] || (offset + i >= tmp_strlen)) {
            offset += i;
            tmp_strlen = 0;
            break;
        }
        if ('"' == data[i]) {
            if (tver) {
                tver = data + i + 1;
                break;
            }
            if (0 == strncmp(key, data + i + 1, strlen(key))) {
                i += strlen(key) + 1 + 1;
                tver = data + i - 1;
                if ('"' != tver[0]) {
                    tver = NULL;
                }
            }
        }
    }

    if (tver) {
        tmp_strlen = index(tver, '"') - tver;
        memcpy(data, tver, tmp_strlen);
        data[tmp_strlen] = '\0';
    } else {
        // 还原路径，然后把路径作为参数再次递归传入
        memcpy(data, tmp_data, strlen((const char *)tmp_data) + 1);
        if (i == data_len) {
            // 没有匹配到数据或没有获取到换行标记也要更新offset值
            offset += data_len;
        }
        ret = -1;
    }

    dial_free(tmp_data);
    tmp_data = NULL;


    if (offset < tmp_strlen && NULL == tver) {
        ret = watch_get_msg_by_path(key, data, data_len, offset);
    }

_end:
    if (tmp_data) {
        dial_free(tmp_data);
        tmp_data = NULL;
    }

    if (file) {
        res_fclose(file);
    }

    return ret;
}

static int watch_get_msg_from_json(char *watch_item, char *value, char *key)
{
    int ret = 0;
    char *sty_suffix = ".sty";
    char *json_suffix = ".json";
    u32 tmp_strlen;
    char tmp_name[64];
    u32 sty_strlen;

    if (watch_item == NULL) {
        return -1;
    }

    sty_strlen = strlen(sty_suffix);
    tmp_strlen = strlen(watch_item);
    strcpy(tmp_name, watch_item);
    strcpy(&tmp_name[tmp_strlen - sty_strlen], json_suffix);
    tmp_name[tmp_strlen - sty_strlen + strlen(json_suffix)] = '\0';
    ret = watch_get_msg_by_path(key, tmp_name, sizeof(tmp_name), 0);
    if (0 == ret) {
        memcpy(value, tmp_name, strlen(tmp_name) + 1);
    }
    return ret;
}

int watch_get_uuid(char *watch_item, char *uuid)
{
    return watch_get_msg_from_json(watch_item, uuid, "prj_uuid");
}

int watch_get_version(char *watch_item, char *version)
{
    return watch_get_msg_from_json(watch_item, version, "version_id");
}

int watch_version_juge(char *watch_item)
{
    char *tver;
    char version[64] = {0};
    if (0 == watch_get_version(watch_item, version)) {
        for (u8 i = 0; i < strlen(WATCH_VERSION_LIST); i += 5) {
            tver = (char *)&WATCH_VERSION_LIST[i];
            if (0 == strncmp(version, tver, strlen(version))) {
                return 0;
            }
        }
    }
    log_error("juge watch version err\n");
    return -1;
}

int standby_watch_set_style(int style)
{
    __this->standby_cur_style = style;
    return style;
}

int standby_watch_get_style()
{
    return __this->standby_cur_style;
}

int watch_get_style()
{
    return __this->dial_cur_style;
}
int watch_set_style(int style)
{
    if (style >= __this->dial_items) {
        return false;
    }
    __this->dial_cur_style = style;
    if (__this->dial_cur_style != get_ui_sys_param(curr_sel_dial)) {
        set_ui_sys_param(curr_sel_dial, __this->dial_cur_style);
        void save_ui_info_to_vm();
        save_ui_info_to_vm();
    }
    return true;
}

int watch_get_items_num()
{
    return __this->dial_items;
}

char *watch_get_full_path()
{
    return __this->dial_path[__this->dial_cur_style];
}

/*比如返回"/WATCH1"*/
char *watch_get_cur_path()
{
    static char path[17];

    char *tmp = __this->dial_path[__this->dial_cur_style];

    u8 find = 0;
    u8 j = 0;
    memset(path, 0, sizeof(path));
    path[j++] = '/';
    for (int i = 0; j < sizeof(path) - 1 ; i++) {
        if (tmp[i] == '/' && find) {
            break;
        }
        if (!tmp[i]) {
            return "NULL";
        }
        if (find) {
            path[j++] = tmp[i];
            continue;
        }
        if (tmp[i] == '/' &&
            (!strncmp(&tmp[i + 1], WATCH_RES_NAME, strlen(WATCH_RES_NAME)) ||
             !strncmp(&tmp[i + 1], WATCH_RES_NAME_SMALL	, strlen(WATCH_RES_NAME_SMALL)))) {
            find = 1;
            continue;
        }
    }
    path[j + 1] = '\0';
    ASCII_ToUpper(path, strlen(path));
    log_debug("<%s> path:%s", __func__, path);
    return path;
}

int watch_set_style_by_name(char *name)
{
    u32 i;
    /* u32 ret; */

    for (i = 0; i < __this->dial_items; i++) {
        if (strncmp(name, __this->dial_path[i], strlen(name)) == 0) {
            watch_set_style(i);
            return 0;
        }
    }

    return -1;
}
char *watch_get_item(int style)
{
    if (style >= __this->dial_items) {
        return NULL;
    }

    return __this->dial_path[style];
}
char *watch_get_root_path()
{
    return RES_PATH;
}
int watch_get_cur_path_len()
{
    return strlen(watch_get_cur_path()) + 1;
}
static u16 watch_name_convert(u8 *utf16, u16 utf16_len, u8 *utf8, u8 bigendian)
{
    u16 len = 0;
    u8 low;
    u8 high;
    u16 wchar;

    while (utf16_len)  {
        low = *utf16;
        utf16++;
        utf16_len--;
        high = *utf16;
        if (bigendian) {
            wchar = (low << 8) + high;
        } else {
            wchar = (high << 8) + low;
        }

        if (wchar <= 0x7F) {
            if (utf8) {
                utf8[len] = (u8)wchar;
            }
            len++;
        } else if (wchar >= 0x80 && wchar <= 0x7FF) {
            if (utf8) {
                utf8[len] = 0xc0 | ((wchar >> 6) & 0x1f);
            }
            len++;
            if (utf8) {
                utf8[len] = 0x80 | (wchar & 0x3f);
            }
            len++;
        } else if (wchar >= 0x800 && wchar < 0xFFFF) {
            if (utf8) {
                utf8[len] = 0xe0 | ((wchar >> 12) & 0x0f);
            }
            len++;
            if (utf8) {
                utf8[len] = 0x80 | ((wchar >> 6) & 0x3f);
            }
            len++;
            if (utf8) {
                utf8[len] = 0x80 | (wchar & 0x3f);
            }
            len++;
        } else if (wchar == 0xFFFF) {
            /* len += 4; */
        } else {
            return -1;
        }
        utf16 ++;
        utf16_len--;
    }
    return len;
}

void ftl_nandflash_get_dirinfo(void *file_buf, u32 *file_num)
{
    struct __dev *ftl_dev = dev_manager_check_by_logo(TCFG_NANDFLASH_UI_FAT_LOGO);
    FILE *file = NULL;
    FS_DIR_INFO dir_info = {0};

    if (!ftl_dev) {
        printf("dev not find !!!logo:%s\n", TCFG_NANDFLASH_UI_FAT_LOGO);
        return ;
    }
    /* printf("dev_root:%s", dev_manager_get_root_path(ftl_dev)); */
    fset_ext_type(dev_manager_get_root_path(ftl_dev), "ALL");
    fopen_dir_info(dev_manager_get_root_path(ftl_dev), &file, 0);
    int total_num = fenter_dir_info(file, &dir_info);
    /* printf("%s total_num:%d", __func__, total_num); */
    *file_num = total_num;
    if (!file_buf) {
        if (file) {
            fclose(file);
        }
        return;
    }
    u8 *fbuf = (u8 *)file_buf;
    u8 tmp_file_name[16];
    int check_cnt = 0;
    //找文件名len<12的文件
    for (int i = 0 ; i < total_num; i++) {
        fget_dir_info(file, i + 1, 1, &dir_info);
        printf("[dir_idx:%d] type:%d len:%d", i, dir_info.fn_type, dir_info.lfn_buf.lfn_cnt);
        put_buf((u8 *)dir_info.lfn_buf.lfn, dir_info.lfn_buf.lfn_cnt);
        if (dir_info.fn_type) {//长文件名
            //转短utf8
            int watch_name_len =  watch_name_convert((u8 *)dir_info.lfn_buf.lfn, dir_info.lfn_buf.lfn_cnt, NULL, 0);
            if (watch_name_len > 12) {
                continue;
            }
            memset(tmp_file_name, 0, 16);
            watch_name_convert((u8 *)dir_info.lfn_buf.lfn, dir_info.lfn_buf.lfn_cnt, tmp_file_name, 0);
        } else {//短文件名
            if (dir_info.lfn_buf.lfn_cnt > 12) {
                continue;
            }
            memcpy(tmp_file_name, dir_info.lfn_buf.lfn, dir_info.lfn_buf.lfn_cnt);
        }
        //符合文件名长度<12.拷贝出去判断
        memcpy(fbuf + check_cnt * 12, tmp_file_name, 12);
        check_cnt++;
    }

    if (file) {
        fclose(file);
    }
}
void file_system_get_dirinfo(void *file_buf, u32 *file_num)
{
#if TCFG_VIRFAT_FLASH_ENABLE
    virfat_flash_get_dirinfo(file_buf, file_num);
#endif
#if TCFG_NANDFLASH_UI_FAT_ENABLE
    ftl_nandflash_get_dirinfo(file_buf, file_num);
#endif
}
void watch_item_sort(void)
{
    u32 i, j, k;
    u32 index = -1;
    u32 file_num;
    char *fname_buf = NULL;
    char *fname = NULL;
    u8 fname_len;
    file_system_get_dirinfo(NULL, &file_num);

    fname_buf = dial_zalloc(file_num * 12);
    if (NULL == fname_buf) {
        goto __watch_add_item_deal_end;
    }

    file_system_get_dirinfo(fname_buf, &file_num);

    for (i = 0, k = 0; i < file_num; i++) {
        fname = fname_buf + i * 12;
        for (j = 0; j < 12; j++) {
            if (' ' == fname[j]) {
                fname[j] = '\0';
                break;
            }
        }
        fname_len = strlen((const char *)fname);
        ASCII_ToLower(fname, fname_len);
        if ((0 == strncmp((const char *)fname, WATCH_RES_NAME, strlen(WATCH_RES_NAME))) || \
            (0 == strncmp((const char *)fname, WATCH_RES_NAME_SMALL, strlen(WATCH_RES_NAME_SMALL)))) {
            fname_len = rindex(__this->dial_path[k], '.') - rindex(__this->dial_path[k], '/') - 1;
            if (0 != strncmp((const char *)fname, rindex(__this->dial_path[k], '/') + 1, fname_len)) {
                // 假如名字不一样，则往下找一样的名字，并进行交换
                for (index = k + 1; index < sizeof(__this->dial_path) / sizeof(__this->dial_path[0]); index++) {
                    fname_len = rindex(__this->dial_path[index], '.') - rindex(__this->dial_path[index], '/') - 1;
                    if (0 == strncmp((const char *)fname, rindex(__this->dial_path[index], '/') + 1, fname_len)) {
                        fname = __this->dial_path[index];
                        for (; index > k; index --) {
                            __this->dial_path[index] = __this->dial_path[index - 1];
                            __this->bgp_related_path[index] = __this->bgp_related_path[index - 1];
                            if (watch_get_style() == (index - 1)) {
                                watch_set_style(index);
                            }
                        }
                        __this->dial_path[k] = fname;
                        break;
                    }
                }
            }
            k++;
        }
    }

__watch_add_item_deal_end:
    if (fname_buf) {
        dial_free(fname_buf);
    }
}

int watch_add_item(char *watch_name)
{
    log_info("%s %s \n", __func__, watch_name);
    char *new_item = NULL;
    /* u32 len; */
    char *suffix = ".sty";

    char *root_path = RES_PATH;
    char watch_path[64];
    u8 watch_name_len = strlen(watch_name);
    ASSERT(((watch_name_len + strlen(root_path) + 1) < sizeof(watch_path)), "err name %s\n", watch_name);

    ASCII_ToLower(watch_name, watch_name_len);
    strcpy(watch_path, root_path);
    if (watch_name[0] == '/') {
        strcpy(&watch_path[strlen(root_path)], &watch_name[1]);
    } else {
        strcpy(&watch_path[strlen(root_path)], &watch_name[0]);
    }
    //path            /  watchxxx . sty \0
    new_item = dial_zalloc(strlen(watch_path) + 1 + watch_name_len + strlen(suffix) + 1);
    if (!new_item) {
        log_error("watch add item fail\n");
        return -1;
    }
    strcat(new_item, watch_path);
    if (watch_name[0] == '/') {
        strcat(new_item, watch_name);
    } else {
        strcat(new_item, "/");
        strcat(new_item, watch_name);
    }
    strcat(new_item, suffix);
    for (u8 i = 0; i < __this->dial_items; i++) {
        if (0 == strcmp(new_item, __this->dial_path[i])) {
            log_info("repeat : %s, %s\n", new_item, __this->dial_path[i]);
            dial_free(new_item);
            return 0;
        }
    }

    __this->dial_path[__this->dial_items] = new_item;
    __this->dial_items++;
    log_info("%s %s succ! __this->dial_items:%d \n", __func__,  new_item, __this->dial_items);

    return 0;
}

int watch_del_item(char *watch_name)
{
    char *suffix = ".sty";
    u8 suffix_len = strlen(suffix);
    u8 watch_name_len = strlen(watch_name);
    int del_succ = 0;
    char *cur_watch = __this->dial_path[__this->dial_cur_style];
    u32 cur_items = __this->dial_items;
    int i, index;

    if (__this->dial_items <= WATCH_ITEMS_MIN_LIMIT) {
        return -1;
    }

    ASCII_ToLower(watch_name, strlen(watch_name));
    for (index = 0 ; index < __this->dial_items; index++) {
        int cur_path_len = strlen(__this->dial_path[index]);
        if ((!cur_path_len) || (cur_path_len < watch_name_len)) {
            continue;
        }
        int path_check_fail = 0;
        for (int j = 0; j < watch_name_len; j++) {
            char path_a = __this->dial_path[index][cur_path_len - suffix_len - j - 1];
            char path_b = watch_name[watch_name_len - j - 1];
            if (path_a != path_b) {
                path_check_fail = 1;
                break;
            }
        }
        if (path_check_fail) {
            continue;
        } else {
            //删除
            if (__this->dial_path[index]) {
                dial_free(__this->dial_path[index]);
                __this->dial_path[index] = NULL;
                __this->bgp_related_path[index] = NULL;
                __this->dial_items--;
                del_succ = 1;
            }
            break;
        }
    }
    if (!del_succ) {
        /* watch_item_sort(); */
        return -1;
    }

    for (i = index; i < cur_items; i++) {
        if (__this->dial_path[i + 1] != NULL) {
            __this->dial_path[i] = __this->dial_path[i + 1];
            __this->bgp_related_path[i] = __this->bgp_related_path[i + 1];
        } else {
            __this->dial_path[i] = NULL;
            __this->bgp_related_path[i] = NULL;
            break;
        }
    }

    for (i = 0; i < __this->dial_items; i++) {
        if (cur_watch == __this->dial_path[i]) {
            log_info("<%s> finish set style %d, %s\n", __func__, i, __this->dial_path[i]);
            watch_set_style(i);
            break;
        }
    }
    if (i == __this->dial_items) {
        log_info("end style\n");
        watch_set_style(0);
    }

    watch_mem_bgp_related();

    for (i = 0; i < __this->dial_items; i++) {
        log_info("current dial_path[%d]:%s", i, __this->dial_path[i]);
    }

    return 0;
}


int ui_dial_res_free()
{
    for (int i = 0; i < WATCH_ITEMS_LIMIT; i++) {
        if (__this->dial_path[i]) {
            dial_free(__this->dial_path[i]);
            __this->dial_path[i] = NULL;
        }
    }
    for (int i = 0; i < BGP_ITEMS_LIMIT; i++) {
        if (__this->bgp_path[i]) {
            dial_free(__this->bgp_path[i]);
            __this->bgp_path[i] = NULL;
        }

    }
    return 0;
}


static int ui_dial_res_init()
{

    log_info("%s \n", __func__);
    ui_dial_res_free();

    __this->dial_cur_style = 0;
    __this->standby_cur_style = -1;
    __this->dial_items = 0;
    __this->bgp_items = 0;

    u32 file_num;
    file_system_get_dirinfo(NULL, &file_num);

    char *fname_buf = dial_zalloc(file_num * 12);
    if (!fname_buf) {
        log_error("[%s]zalloc error,num:%d \n", __func__, file_num);
        return -1;
    }

    file_system_get_dirinfo(fname_buf, &file_num);
    log_debug("<%s> fname_buf:%s", __func__, fname_buf);
    log_info("check file_num:%d \n", file_num);
    int j;
    for (int i = 0; i < file_num; i++) {
        char *fname = &fname_buf[i * 12];
        for (j = 0; j < 12; j++) {
            if (fname[j] == ' ' || fname[j] == '\0') {
                fname[j] = '\0';
                break;
            }
        }
        if (j == 12) {
            log_error("fname overflow\n");
            dial_free(fname_buf);
            return -1;
        }

        u8 fname_len = strlen(fname);
        ASCII_ToLower(fname, fname_len);
        log_info("[index:%d] name:%s \n", i, fname);
        if (strncmp(fname, "watch", strlen("watch")) == 0) {
            watch_add_item(fname);
        } else if (strncmp(fname, "bgp_w", strlen("bgp_w")) == 0) {
#if (defined TCFG_JL_WALLPAPER_ENABLE && TCFG_JL_WALLPAPER_ENABLE)
            void jl_wallpaper_set_by_name(char* name);
            jl_wallpaper_set_by_name(fname);
#else
            watch_bgp_add(fname);
#endif
        }
    }
    dial_free(fname_buf);


    return 0;
}
int ui_dial_manage_init()
{
#if (defined TCFG_JL_WALLPAPER_ENABLE && TCFG_JL_WALLPAPER_ENABLE)
    extern int jl_wallpaper_init();
    jl_wallpaper_init();
#endif
    log_info("%s \n", __func__);
    ui_dial_res_init();
#if (defined TCFG_JL_WALLPAPER_ENABLE && TCFG_JL_WALLPAPER_ENABLE)
#else
    if (watch_bgp_related_init() != 0) {
        log_info("bgp_related_init fail\n");
    } else {
        log_info("bgp_related_init succ\n");
    }
#endif
    /* if (watch_select_read_vm() < 0) { */
    /* __this->dial_cur_style = 0; */
    /* __this->standby_cur_style = -1; */
    /* } */
    return 0;
}
int ui_dial_manage_release()
{
    ui_dial_res_free();
    return  0;
}

int watch_set_init()
{
    ui_dial_manage_init();
    return 0;
}

int switch_ui_page(u8 page, u8 prj)
{
    static u8 last_watch_style = 0xff;
    if ((__this->watch_need_load || (last_watch_style != __this->dial_cur_style)) && !page && (prj == 1)) {
        ui_set_sty_path_by_pj_id(1, NULL);
        ui_set_sty_path_by_pj_id(1, (u8 *)__this->dial_path[__this->dial_cur_style]);

        if (!__this->dial_path[__this->dial_cur_style] || !strlen(__this->dial_path[__this->dial_cur_style])) {
            log_error("watch res is null,watch style =%d\n", __this->dial_cur_style);
            /* ASSERT(0); */
        }
        last_watch_style = __this->dial_cur_style;
        __this->watch_need_load = 0;
    }

#if UI_UPGRADE_RES_ENABLE//升级模式加载资源
    if (app_get_current_mode_name() == APP_MODE_UPDATE ||
        app_get_current_mode_name() == APP_MODE_RCSP) {
        watch_set_need_reload(1);
    }
#endif

    return 0;
}

int watch_select_wtrite_vm(void *priv)
{
    int ret = 0;
    /* if ((int)priv == (int)SYSCFG_WRITE_ERASE_STATUS) { */
    /* dial_sty.watch_style = 0; */
    /* dial_sty.standby_watch_style = -1; */
    /* } */
    /* ret = syscfg_write(VM_WATCH_SELECT, &dial_sty, sizeof(struct __WATCH_STYLE)); */
    /* if (ret != sizeof(struct __WATCH_STYLE)) { */
    /* printf("watch_select_wtrite_vm err\n"); */
    /* return -1; */
    /* } */
    return ret;
}

int watch_select_read_vm()
{
    int ret = 0;
    /* ret = syscfg_read(VM_WATCH_SELECT, &dial_sty, sizeof(struct __WATCH_STYLE)); */
    /* if ((sizeof(struct __WATCH_STYLE) != ret) || */
    /* (dial_sty.watch_style >= watch_items) || */
    /* (dial_sty.standby_watch_style >= watch_items)) { */
    /* printf("write watch_select_read_vm err\n"); */
    /* ret = -1; */
    /* } */
    return ret;
}
/* REGISTER_WATCH_SYSCFG(watch_select_ops) = { */
/* .name = "watch_select", */
/* .read = watch_select_read_vm, */
/* .write = watch_select_wtrite_vm, */
/* }; */

/*检查当前的表盘文件是否正常*/
int ui_watch_resfile_check(void)
{
    if (!__this->dial_items) {
        return false;
    }
    UI_RESFILE *fp_watch;
    for (int i = 0; i < __this->dial_items; i++) {
        fp_watch = res_fopen(__this->dial_path[i], "r");
        if (!fp_watch) {
            log_error("[%s]  check fail!!!", __func__);
            return false;
        }
        extern int sty_file_check(UI_RESFILE * file);
        if (!sty_file_check(fp_watch)) {
            log_error("[%s]path:%d  check fail!!!", __func__, __this->dial_path[i]);
            res_fclose(fp_watch);
            return false;
        }
        res_fclose(fp_watch);
    }
    log_info("<%s> check succ", __func__);
    return true;
}


#if (defined TCFG_JL_WALLPAPER_ENABLE && TCFG_JL_WALLPAPER_ENABLE)
/***********************************************************************
对于电子徽章的特殊需求，单独写一套去管理壁纸资源，不使用手表原有表盘背景的管理
链表管理路径以及配置，节省内存，增删灵活，后续增加的壁纸太多，比如1000张壁纸，
可以将链表分段加载，避免占用过多内存
***********************************************************************/
#define JL_WALLPAPER_PATH_LENGTH                       (32)
// #define WALLPAPER_CONFIG_SIZE                       (32) // 预留给每个图片的配置项，用于以后增加功能
#define JL_WALLPAPER_NUMBER_MAX                        (100)
// #define WALLPAPER_PATH_LEN                          (40)
#define JL_WALLPAPER_CAROUSEL_TIME_INTERVAL            (3000)

#define JL_WALLPAPER_RES_PATH                          "storage/virfat_flash/C/"
#define JL_WALLPAPER_CONFIG_FILE_NAME                  "wp_cfg.bin"

struct _node {
    char wallpaper_path[JL_WALLPAPER_PATH_LENGTH];
    // uint8_t config_reserved[WALLPAPER_CONFIG_SIZE];
    struct _node* prev;
    struct _node* next;
};

typedef struct _linked_list {
    int length;
    struct _node* using;
    struct _node* head;
} linked_list_t;

enum {
    JL_WALLPAPER_CAROUSEL_NULL,
    JL_WALLPAPER_CAROUSEL_SEQUENTIAL,
    JL_WALLPAPER_CAROUSEL_RANDOM,
};

static linked_list_t g_jl_wallpaper_list = {0}; // 写list要enter critical
static void* g_jl_wallpaper_wmem_file = NULL;
static uint8_t g_jl_wallpaper_is_inited = 0;
static uint16_t g_jl_wallpaper_carousel_timer_id = 0;
static uint8_t g_jl_wallpaper_carousel_type = 0;

static void jl_wallpaper_list_dump() {
    printf("\n\n---------------------------------------------------------\n");
    printf("g_jl_wallpaper_list.head %x, g_jl_wallpaper_list.length %d, g_jl_wallpaper_list.using %x\n", \
            g_jl_wallpaper_list.head, g_jl_wallpaper_list.length, g_jl_wallpaper_list.using);
    if (g_jl_wallpaper_list.head) {
        printf("g_jl_wallpaper_list.head->next %x, g_jl_wallpaper_list.head->prev %x, g_jl_wallpaper_list.head->wallpaper_path %s\n", \
            g_jl_wallpaper_list.head->next, g_jl_wallpaper_list.head->prev, g_jl_wallpaper_list.head->wallpaper_path);
    } else {
        printf("g_jl_wallpaper_list.head is null\n");
    }

    if (g_jl_wallpaper_list.using) {
        printf("g_jl_wallpaper_list.using->next %x, g_jl_wallpaper_list.using->prev %x, g_jl_wallpaper_list.using->wallpaper_path %s\n", \
            g_jl_wallpaper_list.using->next, g_jl_wallpaper_list.using->prev, g_jl_wallpaper_list.using->wallpaper_path);
    } else {
        printf("g_jl_wallpaper_list.using is null\n");
    }
    
    printf("g_jl_wallpaper_wmem_file %x\n", g_jl_wallpaper_wmem_file);
    printf("g_jl_wallpaper_is_inited %d\n", g_jl_wallpaper_is_inited);
    printf("g_jl_wallpaper_carousel_timer_id %d\n", g_jl_wallpaper_carousel_timer_id);
    printf("g_jl_wallpaper_carousel_type %d\n", g_jl_wallpaper_carousel_type);
    printf("---------------------------------------------------------\n\n\n");
}

static int jl_wallpaper_load(linked_list_t* list) {
    int index = 0;
    struct _node* p = NULL;
    struct _node* prev = NULL;
    char using_path[JL_WALLPAPER_PATH_LENGTH] = {0};
    printf("-------------------------------%s %d\n", __func__, __LINE__);
    // OS_ENTER_CRITICAL();
    // load path and config data from flash
    if (g_jl_wallpaper_wmem_file) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        fclose(g_jl_wallpaper_wmem_file);
    }printf("-------------------------------%s %d\n", __func__, __LINE__);
    g_jl_wallpaper_wmem_file = fopen(RES_PATH JL_WALLPAPER_CONFIG_FILE_NAME, "r");
    if (!g_jl_wallpaper_wmem_file) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        printf("%s %d null", __func__, __LINE__);
        // OS_EXIT_CRITICAL();
        return -1;
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    memset(&g_jl_wallpaper_list, 0, sizeof(g_jl_wallpaper_list));
printf("-------------------------------%s %d\n", __func__, __LINE__);
    fseek(g_jl_wallpaper_wmem_file, 0, SEEK_SET);printf("-------------------------------%s %d\n", __func__, __LINE__);
    fread(&g_jl_wallpaper_list.length, sizeof(g_jl_wallpaper_list.length), 1, g_jl_wallpaper_wmem_file);printf("-------------------------------%s %d\n", __func__, __LINE__);
    fread(using_path, JL_WALLPAPER_PATH_LENGTH, 1, g_jl_wallpaper_wmem_file);
printf("-------------------------------%s %d\n", __func__, __LINE__);
    using_path[JL_WALLPAPER_PATH_LENGTH - 1] = 0;
    printf("using_path %s\n", using_path);
printf("-------------------------------%s %d\n", __func__, __LINE__);
    for (int i = 0; i < g_jl_wallpaper_list.length; i++) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        p = (struct _node*)malloc(sizeof(struct _node));
        if (!p) {
            printf("%s %d malloc failed", __func__, __LINE__);
            goto err;
        }
printf("-------------------------------%s %d\n", __func__, __LINE__);
        fread(p->wallpaper_path, JL_WALLPAPER_PATH_LENGTH, 1, g_jl_wallpaper_wmem_file);
printf("-------------------------------%s %d\n", __func__, __LINE__);
        if (!g_jl_wallpaper_list.head) {
            g_jl_wallpaper_list.head = p;
            p->prev = NULL;
        } else {
            prev->next = p;
            p->prev = prev;
        }
        p->next = NULL;
        prev = p;
    }
    fclose(g_jl_wallpaper_wmem_file);
    g_jl_wallpaper_wmem_file = NULL;

    p = g_jl_wallpaper_list.head;
    for (int i = 0; p && i < g_jl_wallpaper_list.length; i++) {
        if (!strncmp(using_path, p->wallpaper_path, JL_WALLPAPER_PATH_LENGTH)) {
            g_jl_wallpaper_list.using = p;
            break;
        }
        p = p->next;
    }

    if (!g_jl_wallpaper_list.using) {
        g_jl_wallpaper_list.using = g_jl_wallpaper_list.head;
    }

    jl_wallpaper_list_dump();
    // OS_EXIT_CRITICAL();

    return 0;

err:
    printf("%s %d enter err return", __func__, __LINE__);
    jl_wallpaper_list_dump();
    p = g_jl_wallpaper_list.head;
    while (p) {
        prev = p;
        p = p->next;
        free(prev);
    }
    g_jl_wallpaper_list.head = NULL;
    g_jl_wallpaper_list.length = 0;
    fclose(g_jl_wallpaper_wmem_file);
    g_jl_wallpaper_wmem_file = NULL;
    // OS_EXIT_CRITICAL();

    return -1;
}

static int jl_wallpaper_save(linked_list_t* list) {
    struct _node* p = NULL;
    int i = 0;

    // OS_ENTER_CRITICAL();
    if (g_jl_wallpaper_wmem_file) {
        fdelete(g_jl_wallpaper_wmem_file);
    }
    g_jl_wallpaper_wmem_file = fopen(RES_PATH JL_WALLPAPER_CONFIG_FILE_NAME, "w+");
    if (!g_jl_wallpaper_wmem_file) {
        printf("%s %d null", __func__, __LINE__);
        jl_wallpaper_list_dump();
        // OS_EXIT_CRITICAL();

        return -1;
    }
    // u16 CRC16(const void *ptr, u32 len);
    fseek(g_jl_wallpaper_wmem_file, sizeof(g_jl_wallpaper_list.length)+JL_WALLPAPER_PATH_LENGTH*(g_jl_wallpaper_list.length+1), SEEK_SET);
    fseek(g_jl_wallpaper_wmem_file, 0, SEEK_SET);

    fwrite(&g_jl_wallpaper_list.length, sizeof(g_jl_wallpaper_list.length), 1, g_jl_wallpaper_wmem_file);
    fwrite(g_jl_wallpaper_list.using->wallpaper_path, JL_WALLPAPER_PATH_LENGTH, 1, g_jl_wallpaper_wmem_file);
    
    p = g_jl_wallpaper_list.head;
    for (i = 0; i < g_jl_wallpaper_list.length && p; i++) {
        fwrite(p->wallpaper_path, JL_WALLPAPER_PATH_LENGTH, 1, g_jl_wallpaper_wmem_file);
        p = p->next;
    }
    if (i != g_jl_wallpaper_list.length) {
        printf("error!!! %s %d, i = %d", __func__, __LINE__, i);
    }

    fclose(g_jl_wallpaper_wmem_file);
    g_jl_wallpaper_wmem_file = NULL;
    jl_wallpaper_list_dump();
    // OS_EXIT_CRITICAL();

    return 0;
}

void jl_wallpaper_carousel_func(void* p) {
    uint32_t r = 0;

    switch (g_jl_wallpaper_carousel_type)
    {
    case JL_WALLPAPER_CAROUSEL_NULL:
        sys_timer_del(g_jl_wallpaper_carousel_timer_id);
        g_jl_wallpaper_carousel_timer_id = 0;
        break;
    case JL_WALLPAPER_CAROUSEL_SEQUENTIAL:
        g_jl_wallpaper_list.using = g_jl_wallpaper_list.using->next;
        if (!g_jl_wallpaper_list.using) {
            g_jl_wallpaper_list.using = g_jl_wallpaper_list.head;
        }
        ui_hide(ID_WINDOW_DIAL);
        ui_show(ID_WINDOW_DIAL);
        break;
    case JL_WALLPAPER_CAROUSEL_RANDOM:
        r = rand32() % g_jl_wallpaper_list.length;
        for (int i = 0; i < r; i++) {
            g_jl_wallpaper_list.using = g_jl_wallpaper_list.using->next;
            if (!g_jl_wallpaper_list.using) {
                g_jl_wallpaper_list.using = g_jl_wallpaper_list.head;
            }
        }
        ui_hide(ID_WINDOW_DIAL);
        ui_show(ID_WINDOW_DIAL);
        break;
    
    default:
        break;
    }
    
}

int jl_wallpaper_init() {

    printf("-------------------------------%s %d\n", __func__, __LINE__);
    // load path and config data from flash
    jl_wallpaper_load(&g_jl_wallpaper_list);

    // carousel timer init
    if (g_jl_wallpaper_carousel_timer_id) {
        sys_timer_del(g_jl_wallpaper_carousel_timer_id);
        g_jl_wallpaper_carousel_timer_id = 0;
    }

    if (JL_WALLPAPER_CAROUSEL_NULL != g_jl_wallpaper_carousel_type) {
        g_jl_wallpaper_carousel_timer_id = sys_timer_add(NULL, jl_wallpaper_carousel_func, JL_WALLPAPER_CAROUSEL_TIME_INTERVAL);
    }

    g_jl_wallpaper_is_inited = 1;
    
}

// Uninitialize wallpaper module to ensure system can run GIF and AI modules
int jl_wallpaper_uninit() {

    // free path malloc
    jl_wallpaper_free_list(&g_jl_wallpaper_list);
    
}

int jl_wallpaper_switch_carousel_mode(int mode) {

    if (mode < JL_WALLPAPER_CAROUSEL_NULL || mode > JL_WALLPAPER_CAROUSEL_RANDOM) {
        return -1;
    }

    g_jl_wallpaper_carousel_type = mode;
    if (g_jl_wallpaper_carousel_timer_id) {
        sys_timer_del(g_jl_wallpaper_carousel_timer_id);
        g_jl_wallpaper_carousel_timer_id = 0;
    }

    if (JL_WALLPAPER_CAROUSEL_NULL != g_jl_wallpaper_carousel_type) {    
        g_jl_wallpaper_carousel_timer_id = sys_timer_add(NULL, jl_wallpaper_carousel_func, JL_WALLPAPER_CAROUSEL_TIME_INTERVAL);
    }
    
    return 0;
}

int jl_wallpaper_get_carousel_mode() {
    return g_jl_wallpaper_carousel_type;
}

int jl_wallpaper_add(const char* path, int path_length) {
    struct _node* insert = NULL;
    struct _node* p = NULL;
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (!g_jl_wallpaper_is_inited) {
        log_info("g_jl_wallpaper_is_inited is 0");
        return -1;
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (path_length >= JL_WALLPAPER_PATH_LENGTH) {
        log_info("name_length exceeds WALLPAPER_NAME_LENGTH");
        return -1;
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    insert = (struct _node*)malloc(sizeof(struct _node));
    if (!insert) {
        log_info("%s %d malloc failed", __func__, __LINE__);
        return -1;
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    memset(insert, 0, sizeof(struct _node));
    memcpy(insert->wallpaper_path, path, JL_WALLPAPER_PATH_LENGTH);
printf("-------------------------------%s %d\n", __func__, __LINE__);
    // OS_ENTER_CRITICAL();
    if (!g_jl_wallpaper_list.using) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        g_jl_wallpaper_list.head = insert;printf("-------------------------------%s %d\n", __func__, __LINE__);
        insert->next = NULL;printf("-------------------------------%s %d\n", __func__, __LINE__);
        insert->prev = NULL;printf("-------------------------------%s %d\n", __func__, __LINE__);
    } else {printf("-------------------------------%s %d\n", __func__, __LINE__);
        p = g_jl_wallpaper_list.using->next;printf("-------------------------------%s %d\n", __func__, __LINE__);
        insert->next = p;printf("-------------------------------%s %d\n", __func__, __LINE__);
        insert->prev = g_jl_wallpaper_list.using;printf("-------------------------------%s %d\n", __func__, __LINE__);
        g_jl_wallpaper_list.using->next = insert;printf("-------------------------------%s %d\n", __func__, __LINE__);
        if (p) {
            p->prev = insert;printf("-------------------------------%s %d\n", __func__, __LINE__);
        }
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    g_jl_wallpaper_list.using = insert;
    g_jl_wallpaper_list.length++;
    // OS_EXIT_CRITICAL();
    jl_wallpaper_save(&g_jl_wallpaper_list);
    return 0;
}

int jl_wallpaper_remove_using() {
    struct _node* p = NULL;
    
    // OS_ENTER_CRITICAL();
    p = g_jl_wallpaper_list.using;
    g_jl_wallpaper_list.using = p->prev;
    p->prev->next = p->next;
    p->next->prev = p->prev;
    free(p);
    g_jl_wallpaper_list.length--;
    // OS_EXIT_CRITICAL();

    return 0;
}


char* jl_wallpaper_get_path() {
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (g_jl_wallpaper_list.using) {printf("---------------------------------------------g_jl_wallpaper_list.using %x\n", g_jl_wallpaper_list.using);
        return g_jl_wallpaper_list.using->wallpaper_path;
    }

    return NULL;
}

void jl_wallpaper_set_by_name(char* name) {
    struct _node* p = NULL;
    void* fp = NULL;
    char path_full[JL_WALLPAPER_PATH_LENGTH] = {0};

    if (!name) {
        return;
    }

    if ('/' == name[0]) {
        name++;
    }

printf("-------------------------------%s %d\n", __func__, __LINE__);
    p = g_jl_wallpaper_list.head;
    while (p)
    {printf("-------------------------------%s %d\n", __func__, __LINE__);
        if (!strncmp(p->wallpaper_path, name, JL_WALLPAPER_PATH_LENGTH)) {
            g_jl_wallpaper_list.using = p;
            break;
        }
        p = p->next;
    }
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (!p && strncmp(name, "null", sizeof("null")) && strlen(name)) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        memset(path_full, 0, JL_WALLPAPER_PATH_LENGTH);
        sprintf(path_full, RES_PATH"%s", name);
        printf("-----------------%s\n", path_full);
        if (fp = fopen(path_full, "r")) {printf("-------------------------------%s %d\n", __func__, __LINE__);
            fclose(fp);
            jl_wallpaper_add(path_full, strlen(path_full));
        }
    }
}

void jl_wallpaper_scroll(int delta) {
    struct _node* p = NULL;printf("delta = %d\n", delta);jl_wallpaper_list_dump();
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (!g_jl_wallpaper_list.length) {
        return;
    }
    if (delta > g_jl_wallpaper_list.length) {
        delta = delta % g_jl_wallpaper_list.length;
    }
    
    printf("delta = %d, g_jl_wallpaper_list.length %d\n", delta, g_jl_wallpaper_list.length);
    if (delta < 0) {printf("-------------------------------%s %d\n", __func__, __LINE__);
        for (int i = 0; i > delta; i--) {
            g_jl_wallpaper_list.using = g_jl_wallpaper_list.using->prev;
            if (!g_jl_wallpaper_list.using) {printf("-------------------------------%s %d\n", __func__, __LINE__);
                p = g_jl_wallpaper_list.head;
                while (p->next) {printf("-------------------------------%s %d\n", __func__, __LINE__);
                    p = p->next;
                }
                g_jl_wallpaper_list.using = p;
            }
        }
    } else {printf("-------------------------------%s %d\n", __func__, __LINE__);
        for (int i = 0; i < delta; i++) {printf("-------------------------------%s %d\n", __func__, __LINE__);
            g_jl_wallpaper_list.using = g_jl_wallpaper_list.using->next;
            if (!g_jl_wallpaper_list.using) {printf("-------------------------------%s %d\n", __func__, __LINE__);
                g_jl_wallpaper_list.using = g_jl_wallpaper_list.head;
            }
        }
    }

    jl_wallpaper_save(&g_jl_wallpaper_list);
}

/************************************************************************/
/*                         ble互传部分                                   */
/************************************************************************/
extern int g_mtu;
extern int g_conn_interval;

enum JL_WALLPAPER_FRAME_TYPE {
    JL_WALLPAPER_CONTROLL_FRAME,
    JL_WALLPAPER_DATA_FRAME,
};

enum JL_WALLPAPER_CMD_TYPE {
    JL_WALLPAPER_TRANS_START,
    JL_WALLPAPER_TRANS_STOP,
};


typedef struct _jl_wallpaper_packet_head {
    uint8_t frame_type;
    union sub_type {
        uint8_t cmd_type;
        uint8_t data_seq;
    };
    uint16_t data_length;
} jl_wallpaper_packet_head_t;
extern void *rcsp_server_ble_hdl;
static int jl_wallpaper_send_packet(void* buf, uint16_t data_length) {
    return app_ble_att_send_data(ble_client_get_connect_handle(), ATT_CHARACTERISTIC_aa01_01_VALUE_HANDLE, buf, data_length, ATT_OP_WRITE);
    // return app_ble_att_send_data(rcsp_server_ble_hdl, 0x06, buf, data_length, ATT_OP_WRITE);
    // return app_ble_att_send_data(ble_client_get_connect_handle(), 0x06, buf, data_length, ATT_OP_WRITE_WITHOUT_RESPOND); //ae01_01
    // return JL_CMD_send(JL_OPCODE_CUSTOMER_USER, buf, data_length, 0, ble_client_get_connect_handle(), NULL);
}

static int jl_wallpaper_send_cmd(uint8_t jl_wallpaper_cmd, uint8_t* data, uint16_t data_length) {
    uint8_t* cmd_packet = NULL;
    jl_wallpaper_packet_head_t* cmd_head = NULL;
    
    cmd_packet = malloc(sizeof(jl_wallpaper_packet_head_t) + data_length);
    if (!cmd_packet) {
        printf("error %s %d malloc failed\n", __func__, __LINE__);
        return -1;
    }

    memset(cmd_packet, 0, sizeof(jl_wallpaper_packet_head_t) + data_length);
    cmd_head = (jl_wallpaper_packet_head_t*)cmd_packet;

    cmd_head->frame_type = JL_WALLPAPER_CONTROLL_FRAME;
    cmd_head->cmd_type = jl_wallpaper_cmd;
    cmd_head->data_length = sizeof(jl_wallpaper_packet_head_t) + data_length;

    if (data && data_length) {
        memcpy(cmd_packet + sizeof(jl_wallpaper_packet_head_t), data, data_length);
    }
    
    put_buf(cmd_packet, sizeof(jl_wallpaper_packet_head_t) + data_length);
    return jl_wallpaper_send_packet(cmd_packet, sizeof(jl_wallpaper_packet_head_t) + data_length);
}

int jl_wallpaper_sharing_send() {
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("%s rets %x\n", __func__, rets);

    uint8_t seq = 0;
    uint8_t *packet = NULL;
    void *fp = 0;
    int read_len = 0;
    int packet_buf_len = g_mtu;
    int total_size = 0;
    int len = 0;
    int ret = 0;
    uint8_t wallpaper_trans_cmd_buf[sizeof(total_size) + JL_WALLPAPER_PATH_LENGTH];

    if (!g_jl_wallpaper_list.using) {
        printf("error, local dev has no wallpaper data\n");
        ret = -1;
        goto err;
    }
    
    packet = malloc(packet_buf_len);
    if (!packet) {
        printf("%s %d malloc failed packet_buf_len %d\n", __func__, __LINE__, packet_buf_len);
        ret = -1;
        goto err;
    }

    fp = fopen(g_jl_wallpaper_list.using->wallpaper_path, "r");
    if (!fp) {
        printf("%s %d fp is null\n", __func__, __LINE__);
        ret = -1;
        goto err;
    }
    total_size = flen(fp);

    memset(wallpaper_trans_cmd_buf, 0, sizeof(total_size) + JL_WALLPAPER_PATH_LENGTH);
    memcpy(wallpaper_trans_cmd_buf, &total_size, sizeof(total_size));

    len = strlen(g_jl_wallpaper_list.using->wallpaper_path);
    for (int i = len - 1; i > 0; i--) {
        if (g_jl_wallpaper_list.using->wallpaper_path[i] == '/') {
            memcpy(wallpaper_trans_cmd_buf + sizeof(total_size), g_jl_wallpaper_list.using->wallpaper_path + i + 1, strlen(g_jl_wallpaper_list.using->wallpaper_path + i + 1));
            break;
        }
    }

    
    jl_wallpaper_send_cmd(JL_WALLPAPER_TRANS_START, wallpaper_trans_cmd_buf, sizeof(wallpaper_trans_cmd_buf));
    
    for (int i = 0; i < total_size; ) {
        printf("i = %d total_size %d data_length %d\n",i, total_size, read_len);
        memset(packet, 0, packet_buf_len);
        jl_wallpaper_packet_head_t* packet_head = (jl_wallpaper_packet_head_t*)packet;
        packet_head->frame_type = JL_WALLPAPER_DATA_FRAME;
        packet_head->data_seq = seq;
        
        fseek(fp, i, SEEK_SET);
        read_len = fread(packet + sizeof(jl_wallpaper_packet_head_t), packet_buf_len - sizeof(jl_wallpaper_packet_head_t), 1, fp);
        packet_head->data_length = read_len;

        printf("%s %d read_len = %d packet_buf_len = %d, i = %d, i+read_len = %d\n", __func__, __LINE__, read_len, packet_buf_len, i, i+read_len);
        put_buf(packet, read_len + sizeof(jl_wallpaper_packet_head_t));

        ret = jl_wallpaper_send_packet(packet, read_len + sizeof(jl_wallpaper_packet_head_t));
        printf("ret = %d\n", ret);
        seq++;
        i += read_len;
        
        os_time_dly(g_conn_interval*1.25*3/10);
        // os_time_dly(20);
        wdt_clear();
    }    

    jl_wallpaper_send_cmd(JL_WALLPAPER_TRANS_STOP, NULL, 0);

err:
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    if (packet) {
        free(packet);
        packet = NULL;
    }
    return ret;
}

int jl_wallpaper_sharing_recv(uint8_t *data, int length) {
    jl_wallpaper_packet_head_t* packet_head = NULL;
    jl_wallpaper_packet_head_t head = {0};
    static uint8_t last_seq = 0xff;
    static int total_size = 0;
    static int recv_size = 0;
    static void* recv_file_fp = NULL;
    static char path[JL_WALLPAPER_PATH_LENGTH] = {0};
printf("-------------------------------%s %d\n", __func__, __LINE__);
    if (!data || length < sizeof(jl_wallpaper_packet_head_t)) {
        return -1;
    }
    put_buf(data, length);
    printf("-------------------------------%s %d\n", __func__, __LINE__);
    memcpy(&head, data, sizeof(head));

    packet_head = &head;
printf("-------------------------------%s %d\n", __func__, __LINE__);
    printf("------------------%s\n", os_current_task());
    printf("%s %d length %d\n", __func__, __LINE__, length);
    int l = packet_head->data_length;
    int t = packet_head->frame_type;
    printf(" packet_head->data_length %d ,packet_head->frame_type %d\n", l, t);
    switch (packet_head->frame_type)
    {
    case JL_WALLPAPER_CONTROLL_FRAME:
        switch (packet_head->cmd_type)
        {
        case JL_WALLPAPER_TRANS_START:
            uint8_t* value = data + sizeof(jl_wallpaper_packet_head_t);
            
            total_size = value[0] | value[1] << 8 | value[2] << 16 | value[3] << 24;
            printf("value = %x-%x-%x-%x\n", value[0], value[1], value[2], value[3]);
            printf("size %d, 0x%x\n", total_size, total_size);
            
            if (recv_file_fp) {
                printf("fp has not closed\n");
                fclose(recv_file_fp);
            }

            memset(path, 0, JL_WALLPAPER_PATH_LENGTH);
            snprintf(path, JL_WALLPAPER_PATH_LENGTH, RES_PATH"%s", value + 4);
            recv_file_fp = fopen(path, "r");
            // todo for file name repetition
            if (recv_file_fp) {
                printf("file has existed\n");
                // file has existed, should choose to rename new file or delete original file(fdelete)
                fdelete(recv_file_fp);
            }
            

            recv_file_fp = fopen(path, "w+");
            if (!recv_file_fp) {
                printf("%s %d file open err\n", __func__, __LINE__);
                return -1;
            }
            
            fseek(recv_file_fp, total_size, SEEK_SET);
            fseek(recv_file_fp, 0, SEEK_SET);
            recv_size = 0;
            last_seq = 0xff;

            UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_UPGRADE, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_UPGRADE);
            UI_MSG_POST("upgrade:wait=%4", 3);
            break;

        case JL_WALLPAPER_TRANS_STOP:
            if (recv_file_fp) {
                fclose(recv_file_fp);
                recv_file_fp = NULL;
            }

            for (int i = strlen(path) - 1; i > 0; i--) {
                if (path[i] == '/') {
                    jl_wallpaper_set_by_name(path + i);
                    break;
                }
            }

            UI_SHOW_WINDOW(ID_WINDOW_DIAL);
            break;
        default:
            break;
        }
        break;
    case JL_WALLPAPER_DATA_FRAME:
        if (!recv_file_fp) {
            printf("error file has not created\n");
            return -1;
        }
        uint8_t* value = data + sizeof(jl_wallpaper_packet_head_t);
        printf("seq = 0x%02X, last_seq = %02X\n", packet_head->data_seq, last_seq);
        last_seq++;
        if (packet_head->data_seq != last_seq) {
            ASSERT(0);
        }
        printf("write start %llu\n", jiffies_usec());
        int ret = fwrite(value, packet_head->data_length, 1, recv_file_fp);
        printf("write end %llu\n", jiffies_usec());
        if (!ret) {
            printf("%s %d -ret !!!!!!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__);
        }
        UI_MSG_POST("upgrade:process=%4", recv_size*100/total_size);
        recv_size += ret;
        printf("ret = %d, recv_size = %d, total_size %d\n", ret, recv_size, total_size);
        break;
    default:
        break;
    }
    
}


/************************************************************************/


#endif

#endif
