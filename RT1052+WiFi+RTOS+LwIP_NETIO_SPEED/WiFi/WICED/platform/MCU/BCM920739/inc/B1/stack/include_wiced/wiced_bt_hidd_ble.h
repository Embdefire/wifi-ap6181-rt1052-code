/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 * Human Interface Device Profile (HID) Device over BLE
 *
 */
#pragma once

#include "wiced_bt_fw.h"
#include "wiced_bt_dev.h"
#include "hiddefs.h"


/******************************************************
 *              Constants
 ******************************************************/

/* HID-LE-Device Callback Events */
enum wiced_bt_hidd_ble_cback_event_e
{
    WICED_BT_HIDD_BLE_DEV_EVT_OPEN,        /**< Connected to host with Interrupt and Control  Data = 1 if Virtual Cable
                                                 Channels in OPEN state. pdata = Host BD-Addr.*/

    WICED_BT_HIDD_BLE_DEV_EVT_CLOSE,       /**< Connection with host is closed.     Data=Reason Code. */
    WICED_BT_HIDD_BLE_DEV_EVT_GET_REPORT,  /**< Host sent GET_REPORT                Data=Length pdata=structure
                                                 having details of get-report. */
    WICED_BT_HIDD_BLE_DEV_EVT_SET_REPORT,  /**< Host sent SET_REPORT                Data=Length pdata=details. */
    WICED_BT_HIDD_BLE_DEV_EVT_GET_PROTO,   /**< Host sent GET_PROTOCOL              Data=NA */
    WICED_BT_HIDD_BLE_DEV_EVT_SET_PROTO,   /**< Host sent SET_PROTOCOL              Data=1 for Report, 0 for Boot */
    WICED_BT_HIDD_BLE_DEV_EVT_DATA         /**< General event data */
};
typedef uint8_t wiced_bt_hidd_ble_cback_event_t;   /**< HIDD BLE callback events */

/* GATT  application error code for HID profile */
#define HIDD_LE_RPT_NOT_SUPT    0x8F    /**< Report not supported */

/* HIDD type */
#define HIDD_LE_KB_TYPE         0x01    /**< bit 0 */
#define HIDD_LE_MICE_TYPE       0x02    /**< bit 1 */
#define HIDD_LE_OTHER_TYPE      0x80    /**< bit 7 */
typedef uint8_t wiced_bt_hidd_ble_dev_t;    /**< HIDD BLE device types */

#define HIDD_LE_PROTO_MODE_RPT        0x00  /**< Report protocol */
#define HIDD_LE_PROTO_MODE_BOOT       0x01  /**< Boot protocol */
typedef uint8_t wiced_bt_hidd_ble_proto_t;  /**< HIDD BLE protocol types */

/* LE HIDD report type */
#define HID_LE_RPT_TYPE_INPUT       0x01    /**< Input reports */
#define HID_LE_RPT_TYPE_OUTPUT      0x02    /**< Output reports */
#define HID_LE_RPT_TYPE_FEATURE     0x03    /**< Feature reports */
#define HID_LE_RPT_TYPE_KB_INPUT    0x04    /**< Keyboard input */
#define HID_LE_RPT_TYPE_KB_OUTPUT   0x05    /**< Keyboard output */
#define HID_LE_RPT_TYPE_MI_INPUT    0x06    /**< Mouse input */
typedef uint8_t   wiced_bt_hidd_ble_rpt_t;  /**< HIDD BLE report types */

#define HID_LE_RPT_TYPE_MAX         HID_LE_RPT_TYPE_FEATURE /**< Maximun report type */

/******************************************************
 *              Type Definitions
 ******************************************************/
enum wiced_bt_hidd_ble_status
{
    WICED_BT_HIDD_BLE_SUCCESS,                      /**< Success */
    WICED_BT_HIDD_BLE_ERR_NOT_REGISTERED,           /**< Not registered */
    WICED_BT_HIDD_BLE_ERR_ALREADY_REGISTERED,       /**< Alreadu registered */
    WICED_BT_HIDD_BLE_ERR_NO_RESOURCES,             /**< No resources */
    WICED_BT_HIDD_BLE_ERR_NO_CONNECTION,            /**< Not connection */
    WICED_BT_HIDD_BLE_ERR_INVALID_PARAM,            /**< Invalid parameter */
    WICED_BT_HIDD_BLE_ERR_UNSUPPORTED,              /**< Not supported */
    WICED_BT_HIDD_BLE_ERR_UNKNOWN_COMMAND,          /**< Unknown command */
    WICED_BT_HIDD_BLE_ERR_CONGESTED,                /**< Congested */
    WICED_BT_HIDD_BLE_ERR_CONN_IN_PROCESS,          /**< Connection in process */
    WICED_BT_HIDD_BLE_ERR_ALREADY_CONN,             /**< Already connected */
    WICED_BT_HIDD_BLE_ERR_DISCONNECTING,            /**< Disconnecting is process */
    WICED_BT_HIDD_BLE_ERR_SET_CONNABLE_FAIL,        /**< Set connectable failiure */
    /* Device specific error codes */
    WICED_BT_HIDD_BLE_ERR_HOST_UNKNOWN,             /**< Host unknown */
    WICED_BT_HIDD_BLE_ERR_L2CAP_FAILED,             /**< L2CAP failed */
    WICED_BT_HIDD_BLE_ERR_AUTH_FAILED,              /**< Authentication failed */
    WICED_BT_HIDD_BLE_ERR_SDP_BUSY,                 /**< SDP busy */
    WICED_BT_HIDD_BLE_ERR_GATT,                     /**< GATT */

    WICED_BT_HIDD_BLE_ERR_INVALID = 0xFF            /**< Invalid */
};
typedef uint8_t wiced_bt_hidd_ble_status_t;         /**< HIDD BLE status codes */

/* report reference descriptor value */
typedef struct
{
    uint8_t                 rpt_id;     /**< Report ID */
    wiced_bt_hidd_ble_rpt_t rpt_type;   /**< Report type */
} wiced_bt_hidd_ble_rpt_ref_t;          /**< HIDD BLE report reference */

/* LE HIDD registration information */
typedef struct
{
    wiced_bt_hidd_ble_dev_t         dev_type;           /**< Device type */
    uint8_t                         num_rpt;            /**< Number of reports */
    uint16_t                        battery_handle;     /**< Battery handle */
    wiced_bt_hidd_ble_rpt_ref_t     *p_rpt_lst;         /**< Pointer to the report reference */
    wiced_bt_hidd_ble_proto_t       proto_cap;          /**< Protocol capability */

} wiced_bt_hidd_ble_dev_info_t;                         /**< HIDD BLE device info */

#define HIDD_LE_REMOTE_WAKE   0x01          /**< Remote wake */
#define HIDD_LE_NORMAL_CONN   0x02          /**< Normally connectable */

typedef struct
{
    uint16_t    dl_len;                     /**< Description length */
    uint8_t     *dsc_list;                  /**< Pointer to the description */
} wiced_bt_hidd_ble_dscp_info_t;            /**< HIDD BLE description info */

/* LE HIDD report map info */
typedef struct
{
    uint16_t                        bcdHID;         /**< HID info in BCD format */
    uint8_t                         contry_code;    /**< Country code */
    uint8_t                         flags;          /**< HID info in BCD format */
    wiced_bt_hidd_ble_dscp_info_t   rpt_map;        /**< Report map */
} wiced_bt_hidd_ble_rpt_map_info_t;                 /**< HIDD BLE report map info */

#define HIDD_REPT_ID_BOOT_KB        1
#define HIDD_REPT_ID_BOOT_MOUSE     2

typedef struct
{
    uint16_t        event;              /**< event */
    uint16_t        len;                /**< length */
    uint16_t        offset;             /**< offset */
    uint16_t        layer_specific;     /**< lay_specific */
} wiced_bt_hidd_bt_hdr_t;               /**< General data in BT_HDR type */

/* LE HIDD report data */
typedef struct
{
    wiced_bt_hidd_bt_hdr_t  hdr;        /**< report data, assuming the first byte of data is report ID */
    uint8_t                 rpt_id;     /**< report ID */
} wiced_bt_hidd_ble_rpt_data_t;         /**< HIDD BLE report data */

/* LE HIDD get report data */
typedef struct
{
    uint8_t rep_type;               /**< HIDD BLE report type */
    uint8_t rep_id;                 /**< HIDD BLE report ID */
} wiced_bt_hidd_ble_get_rpt_data_t; /**< HIDD BLE get report data */

/* LE HIDD cback data */
typedef union
{
    wiced_bt_device_address_t           host_bdaddr;    /**< Host BD-ADDR */
    wiced_bt_hidd_ble_get_rpt_data_t    get_rpt;        /**< Get report */
    wiced_bt_hidd_ble_rpt_data_t        *p_buffer;      /**< General report data */
} wiced_bt_hidd_ble_cback_data_t;                       /**< HIDD BLE callback data */

/**
 * HIDD LE callback
 *
 * Callback for Human Interface Device Profile Device (HIDD)
 *
 * @param[in] event         : Callback event
 * @param[in] data          : Integer data corresponding to the event
 * @param[in] p_data        : Callback data
 *
 * @return void
 */
typedef void (wiced_bt_hidd_ble_cback_t) (uint8_t  event,
                                          uint32_t data,
                                          wiced_bt_hidd_ble_cback_data_t *p_data );
/* HIDD LE registration info */
typedef struct
{
    wiced_bt_device_address_t       host_addr;      /**< Host BD-ADDR */
    wiced_bt_hidd_ble_dev_info_t    dev_info;       /**< Device info */
    wiced_bt_hidd_ble_cback_t       *app_cback;     /**< Callback function */
} wiced_bt_hidd_ble_reg_info_t;                     /**< HIDD BLE registration info */


/**
 *  @addtogroup  hidd_le_api_functions      HIDD over BLE
 *  @ingroup     wicedbt
 *
 *  HIDD LE Functions
 *
 *  @{
 */

/******************************************************
 *              Function Declarations
 ******************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Function         wiced_bt_hidd_ble_init
 *
 *                  Initialize HIDD LE control block and trace variable.
 *
 *  @param[in]      None
 *  @param[out]     None
 *
 *  @return         None
 */
void wiced_bt_hidd_ble_init (void);

/**
 * Function         wiced_bt_hidd_ble_register
 *
 *                  This function must be called at startup to register info related
 *                  to HIDD over LE.
 *
 *
 *  @param[in]      p_reg_info  : SCO index to remove
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_register(wiced_bt_hidd_ble_reg_info_t *p_reg_info);

/**
 * Function         wiced_bt_hidd_ble_deregister
 *
 *                  Disable HIDD service.
 *
 *  @param[in]      None
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_deregister(void);

/**
 * Function         wiced_bt_hidd_ble_connect
 *
 *                  Initiates a connection to the host.
 *
 *  @param[in]      None
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_connect(void);

/**
 * Function         wiced_bt_hidd_ble_disconnect
 *
 *                  Disconnects from the host.
 *
 *  @param[in]      None
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_disconnect(void);

/**
 * Function         wiced_bt_hidd_ble_send_report
 *
 *                  Sends report data to the host.
 *
 *  @param[in]      rep_type    : Report type
 *  @param[in]      rep_id      : Report ID
 *  @param[in]      len         : Length of the data
 *  @param[in]      offset      : Offset of the data
 *  @param[in]      p_rpt       : Pointer to the report data
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_send_report(uint8_t rep_type, uint8_t rpt_id,
                                                         uint16_t len, uint16_t offset, uint8_t *p_rpt);

/**
 * Function         wiced_bt_hidd_ble_hand_shake
 *
 *                  Acks a set report request
 *
 *  @param[in]      status code (see #wiced_bt_hidd_ble_status_t)
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_hand_shake(wiced_bt_hidd_ble_status_t status);

/**
 * Function         wiced_bt_hidd_ble_rsp_get_protocol
 *
 *                  Responds to a get protocol mode request
 *
 *  @param[in]      cur_mode    : Current protocol
 *  @param[out]     None
 *
 *  @return         status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_rsp_get_protocol(wiced_bt_hidd_ble_proto_t cur_mode);

/**
 * Function         wiced_bt_hidd_ble_set_rsp_map_info
 *
 *                  This function shall be called at startup to configure the
 *                  device HID information and report map
 *
 *
 *  @param[in]      p_dev_info  : Device map info
 *  @param[out]     None
 *
 *  @return     status code (see #wiced_bt_hidd_ble_status_t)
 */
wiced_bt_hidd_ble_status_t wiced_bt_hidd_ble_set_rsp_map_info(wiced_bt_hidd_ble_rpt_map_info_t *p_dev_info);

#ifdef __cplusplus
}
#endif

/** @} wicedbt_hidd_ble */
