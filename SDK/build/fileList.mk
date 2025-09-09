
































objs += $(ROOT)/audio/framework/plugs/source/a2dp_file.o $(ROOT)/audio/framework/plugs/source/a2dp_streamctrl.o $(ROOT)/audio/framework/plugs/source/esco_file.o $(ROOT)/audio/framework/plugs/source/adc_file.o $(ROOT)/audio/framework/nodes/esco_tx_node.o $(ROOT)/audio/framework/nodes/plc_node.o $(ROOT)/audio/framework/nodes/volume_node.o
objs += $(ROOT)/audio/framework/nodes/ns_node.o


objs += $(ROOT)/audio/framework/nodes/channle_swap_node.o


objs += $(ROOT)/audio/framework/nodes/effect_dev0_node.o

objs += $(ROOT)/audio/framework/nodes/effect_dev1_node.o

objs += $(ROOT)/audio/framework/nodes/effect_dev2_node.o

objs += $(ROOT)/audio/framework/nodes/effect_dev3_node.o

objs += $(ROOT)/audio/framework/nodes/effect_dev4_node.o

objs += $(ROOT)/audio/framework/nodes/sink_dev0_node.o

objs += $(ROOT)/audio/framework/nodes/sink_dev1_node.o


objs += $(ROOT)/audio/framework/plugs/source/source_dev0_file.o

objs += $(ROOT)/audio/framework/plugs/source/source_dev1_file.o


objs += $(ROOT)/audio/framework/nodes/agc_node.o


objs += $(ROOT)/audio/framework/nodes/surround_demo_node.o


objs += $(ROOT)/audio/framework/nodes/cvp_sms_node.o


objs += $(ROOT)/audio/framework/nodes/cvp_dms_node.o


objs += $(ROOT)/audio/framework/nodes/cvp_3mic_node.o


objs += $(ROOT)/audio/framework/nodes/cvp_develop_node.o


objs += $(ROOT)/audio/framework/nodes/uart_node.o


objs += $(ROOT)/audio/framework/nodes/dns_node.o


objs += $(ROOT)/audio/framework/nodes/ai_tx_node.o $(ROOT)/audio/framework/nodes/a2dp_tx_node.o



objs += $(ROOT)/audio/framework/nodes/ai_rx_file.o $(ROOT)/audio/interface/player/ai_rx_player.o



objs += $(ROOT)/audio/framework/nodes/data_export_node.o
objs += $(ROOT)/audio/test_tools/mic_dut_process.o


objs += $(ROOT)/audio/test_tools/audio_enc_mpt_self.o $(ROOT)/audio/test_tools/audio_enc_mpt_cvp_ctr.o



objs += $(ROOT)/audio/test_tools/cvp_tool.o


objs += $(ROOT)/audio/test_tools/audio_dut_control.o $(ROOT)/audio/test_tools/audio_dut_control_old.o



objs += $(ROOT)/audio/common/audio_node_config.o $(ROOT)/audio/common/audio_dvol.o $(ROOT)/audio/common/audio_general.o $(ROOT)/audio/common/audio_build_needed.o $(ROOT)/audio/common/online_debug/aud_data_export.o $(ROOT)/audio/common/online_debug/audio_online_debug.o $(ROOT)/audio/common/online_debug/audio_capture.o $(ROOT)/audio/common/audio_plc.o $(ROOT)/audio/common/audio_noise_gate.o $(ROOT)/audio/common/audio_ns.o $(ROOT)/audio/common/audio_utils.o $(ROOT)/audio/common/audio_export_demo.o $(ROOT)/audio/common/amplitude_statistic.o $(ROOT)/audio/common/frame_length_adaptive.o $(ROOT)/audio/common/bt_audio_energy_detection.o $(ROOT)/audio/common/audio_event_handler.o $(ROOT)/audio/common/debug/audio_debug.o $(ROOT)/audio/common/audio_volume_mixer.o
objs += $(ROOT)/audio/common/online_debug/aud_mic_dut.o


objs += $(ROOT)/audio/common/uartPcmSender.o
objs += $(ROOT)/audio/interface/player/tone_player.o $(ROOT)/audio/interface/player/ring_player.o $(ROOT)/audio/interface/player/a2dp_player.o $(ROOT)/audio/interface/player/esco_player.o $(ROOT)/audio/interface/player/key_tone_player.o $(ROOT)/audio/interface/player/dev_flow_player.o $(ROOT)/audio/interface/player/adda_loop_player.o $(ROOT)/audio/interface/player/reference_time.o
objs += $(ROOT)/audio/interface/recoder/esco_recoder.o $(ROOT)/audio/interface/recoder/ai_voice_recoder.o $(ROOT)/audio/interface/recoder/dev_flow_recoder.o





objs += $(ROOT)/audio/effect/eq_config.o $(ROOT)/audio/effect/spk_eq.o $(ROOT)/audio/effect/audio_voice_changer_api.o $(ROOT)/audio/effect/esco_ul_voice_changer.o $(ROOT)/audio/effect/bass_treble.o $(ROOT)/audio/effect/audio_dc_offset_remove.o $(ROOT)/audio/effect/effects_adj.o $(ROOT)/audio/effect/effects_default_param.o $(ROOT)/audio/effect/node_param_update.o
objs += $(ROOT)/audio/smart_voice/smart_voice_core.o $(ROOT)/audio/smart_voice/smart_voice_config.o $(ROOT)/audio/smart_voice/jl_kws_platform.o $(ROOT)/audio/smart_voice/user_asr.o $(ROOT)/audio/smart_voice/voice_mic_data.o $(ROOT)/audio/smart_voice/kws_event.o $(ROOT)/audio/smart_voice/nn_vad.o
objs += $(ROOT)/audio/jl_kws/jl_kws_main.o $(ROOT)/audio/jl_kws/jl_kws_audio.o $(ROOT)/audio/jl_kws/jl_kws_algo.o $(ROOT)/audio/jl_kws/jl_kws_event.o
objs += $(ROOT)/audio/CVP/audio_cvp.o $(ROOT)/audio/CVP/audio_cvp_dms.o $(ROOT)/audio/CVP/audio_cvp_3mic.o $(ROOT)/audio/CVP/audio_cvp_online.o $(ROOT)/audio/CVP/audio_cvp_demo.o $(ROOT)/audio/CVP/audio_cvp_develop.o $(ROOT)/audio/CVP/audio_cvp_sync.o $(ROOT)/audio/CVP/audio_cvp_ais_3mic.o $(ROOT)/audio/CVP/audio_cvp_ref_task.o
objs += $(ROOT)/audio/interface/player/tws_tone_player.o
objs += $(ROOT)/audio/interface/player/file_player.o



objs += $(ROOT)/audio/interface/recoder/file_recorder.o



objs += $(ROOT)/audio/framework/plugs/source/linein_file.o


objs += $(ROOT)/audio/framework/plugs/source/pc_spk_file.o $(ROOT)/audio/interface/player/pc_spk_player.o




objs += $(ROOT)/audio/framework/nodes/pc_mic_node.o $(ROOT)/audio/interface/recoder/pc_mic_recoder.o




objs += $(ROOT)/audio/effect/audio_pitch_speed_api.o
objs += $(ROOT)/apps/common/third_party_profile/jieli/online_db/spp_online_db.o $(ROOT)/apps/common/third_party_profile/jieli/online_db/online_db_deal.o $(ROOT)/apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.o $(ROOT)/apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.o $(ROOT)/apps/common/third_party_profile/wechat_sport/wechat_sport_demo.o $(ROOT)/apps/common/third_party_profile/multi_protocol_common.o $(ROOT)/apps/common/third_party_profile/multi_protocol_event.o $(ROOT)/apps/common/third_party_profile/multi_protocol_main.o
objs += $(ROOT)/apps/common/third_party_profile/common/3th_profile_api.o $(ROOT)/apps/common/third_party_profile/common/custom_cfg.o $(ROOT)/apps/common/third_party_profile/common/mic_rec.o





objs += $(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_common.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_server.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_client.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/adv/ble_rcsp_adv.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_config.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_task.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_event.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_manage.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_bt_manage.o
objs += $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve_no_respone.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_respone.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve_no_respone.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_respone.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_user.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/rcsp_command.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_switch_device.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_opt.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_sync.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_adv_bluetooth.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_screen_box_earphone_info.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_eq_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_high_low_vol_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_music_info_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_vol_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_color_led_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_eq_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice_key.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_hearing_aid_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_bt_name_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_key_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_led_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_mic_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_time_stamp_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_work_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_adaptive_noise_reduction.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_ai_no_pick.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_scene_noise_reduction.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_wind_noise_detection.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_voice_enhancement_mode.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_1t2_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_reverbration_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_drc_setting.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_feature.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_status.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_fm_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_rtc_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_music_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_linein_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_bt_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update_tws.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_ch_loader_download.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_cmd.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_opt.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_delete.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/dev_format.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_trans_back.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_bluk_trans_prepare.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_simple_transfer.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensors_data_opt.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_func.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensor_log_notify.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/nfc_data_opt.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_opt.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_sync.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/browser/rcsp_browser.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_watch_info.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_watch_device_config.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_common_info_res_file_handler.o
objs += $(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve_no_response.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_response.o $(ROOT)/apps/common/third_party_profile/jieli/rcsp/client/rcsp_m_update/rcsp_update_master.o





objs += $(ROOT)/apps/common/third_party_profile/jieli/screen_trans/screen_ear_protocol_received.o $(ROOT)/apps/common/third_party_profile/jieli/screen_trans/smartbox_user_app.o




objs += $(ROOT)/apps/common/third_party_profile/swift_pair/swift_pair_protocol.o



objs += $(ROOT)/apps/common/third_party_profile/custom_protocol_demo/custom_protocol.o




objs += $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy.o $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_fmna.o $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_ota.o $(ROOT)/apps/common/third_party_profile/bt_fmy/ble_fmy_modet.o







objs += $(ROOT)/apps/common/third_party_profile/interface/app_protocol_api.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_ota.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_common.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_gma.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_dma.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_mma.o $(ROOT)/apps/common/third_party_profile/interface/app_protocol_tme.o
objs += $(ROOT)/apps/common/third_party_profile/alipay/le_alipay.o
objs += $(ROOT)/apps/common/device/usb/usb_config.o $(ROOT)/apps/common/device/usb/device/descriptor.o $(ROOT)/apps/common/device/usb/device/usb_device.o $(ROOT)/apps/common/device/usb/device/user_setup.o $(ROOT)/apps/common/device/usb/device/task_pc.o







objs += $(ROOT)/apps/common/device/usb/device/msd.o



objs += $(ROOT)/apps/common/device/usb/device/msd_upgrade.o


objs += $(ROOT)/apps/common/device/usb/device/hid.o



objs += $(ROOT)/apps/common/device/usb/device/custom_hid.o $(ROOT)/apps/common/device/usb/device/rcsp_hid_inter.o





objs += $(ROOT)/apps/common/device/usb/device/uac_stream.o


objs += $(ROOT)/apps/common/device/usb/device/uac1.o


objs += $(ROOT)/apps/common/device/usb/device/uac2.o





objs += $(ROOT)/apps/common/device/usb/device/cdc.o




objs += $(ROOT)/apps/common/device/usb/usb_host_config.o $(ROOT)/apps/common/device/usb/host/usb_bulk_transfer.o $(ROOT)/apps/common/device/usb/host/usb_ctrl_transfer.o $(ROOT)/apps/common/device/usb/host/usb_host.o







objs += $(ROOT)/apps/common/device/usb/host/usb_storage.o



objs += $(ROOT)/apps/common/device/usb/host/adb.o




objs += $(ROOT)/apps/common/device/usb/host/aoa.o




objs += $(ROOT)/apps/common/device/usb/host/hid.o





objs += $(ROOT)/apps/common/device/usb/usb_epbuf_manager.o $(ROOT)/apps/common/device/usb/usb_task.o
objs += $(ROOT)/audio/cpu/br35/audio_setup.o $(ROOT)/audio/cpu/br35/audio_config.o
objs += $(ROOT)/audio/cpu/br35/audio_demo/audio_adc_demo.o $(ROOT)/audio/cpu/br35/audio_demo/audio_dac_demo.o





objs += $(ROOT)/apps/watch/ble/bt_ble.o $(ROOT)/apps/watch/ble/ble_adv.o




objs += $(ROOT)/apps/watch/battery/charge.o $(ROOT)/apps/watch/battery/charge_device_handle.o



objs += $(ROOT)/apps/watch/battery/battery_level.o



objs += $(ROOT)/apps/watch/battery/charge_store.o




objs += $(ROOT)/apps/watch/mode/bt/bt_call_kws_handler.o $(ROOT)/apps/watch/mode/bt/kws_voice_event_deal.o




objs += $(ROOT)/apps/watch/mode/mic_effect/mic_effect.o



objs += $(ROOT)/apps/watch/mode/pc/pc.o $(ROOT)/apps/watch/mode/pc/pc_app_msg_handler.o $(ROOT)/apps/watch/mode/pc/pc_key_msg_table.o





objs += $(ROOT)/apps/watch/mode/record/record.o $(ROOT)/apps/watch/mode/record/record_app_msg_handler.o $(ROOT)/apps/watch/mode/record/record_key_msg_table.o





objs += $(ROOT)/apps/watch/mode/rtc/rtc.o $(ROOT)/apps/watch/mode/rtc/rtc_app_msg_handler.o $(ROOT)/apps/watch/mode/rtc/rtc_key_msg_table.o $(ROOT)/apps/watch/mode/rtc/alarm_api.o $(ROOT)/apps/watch/mode/rtc/alarm_user.o
