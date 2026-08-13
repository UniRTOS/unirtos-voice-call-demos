/*****************************************************************/ /**
* @file voice_call_demo.c
* @brief
* @author elmer.tang@quectel.com
* @date 2025-12-12
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2025-12-12 <td>1.0 <td>elmer.tang <td> Init
* </table>
**********************************************************************/
#include "qosa_def.h"
#include "qosa_log.h"
#include "qosa_urc.h"
#include "qosa_dev1.h"
#include "qosa_iic.h"
#include "qosa_gpio.h"
#include "qosa_ims.h"
#include "qosa_event_notify.h"
#include "qosa_voice_call.h"
#include "qosa_pinctrl.h"
#include "qosa_datacall.h"       
#include "qosa_sockets.h"        /* inet_ntop */
#include "voice_call_demo.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG LOG_TAG_DEMO

/**
 *
 * @struct unir_ims_demo_ims_msg_t
 * @brief Message reported by the ims module
 *
 */
typedef struct at_ims_msg
{
    qosa_uint32_t event_id; /*!< Event ID */
    union
    {
        qosa_ims_ring_event_t       ring_event;       /*!< command params cmd QOSA_EVENT_MODEM_IMS_RING_STATUS */
        qosa_ims_disconnect_event_t disconnect_event; /*!< command params cmd QOSA_EVENT_MODEM_IMS_DISCONNECT_STATUS */
        qosa_ims_conn_id_event_t    conn_id_event;    /*!< command params cmd QOSA_EVENT_MODEM_IMS_CONN_ID_EVNET */
    };

} unir_ims_demo_ims_msg_t;

/** Voice call demo task handle */
qosa_task_t g_vc_task = QOSA_NULL;

/** RING semaphore - released in callback, waited in task */
static qosa_sem_t g_ring_sem = QOSA_NULL;

/**
 * @brief Audio codec initialization function
 *
 * This function performs the initialization configuration of the ES8311 audio codec,
 * including the setting of pin functions, the initialization of the I2C interface,
 * as well as the configuration of the internal registers of the codec.
 *
 * @param NONE
 *
 * @return Initialize the result. A value of 0 indicates success, while any non-zero value indicates failure.
 */
static int unir_codec_init()
{
    qosa_int32_t     ret = 0;
    unir_codec_reg_t reg_list[] = ES8311_INIT_REG;
    // pin set
    qosa_pin_set_func(UNIR_IIC_SCL_NUM, UNIR_IIC_FUNC);
    qosa_pin_set_func(UNIR_IIC_SDA_NUM, UNIR_IIC_FUNC);

    // initialize IIC
    ret = qosa_i2c_init(QOSA_I2C_1, QOSA_IIC_STANDARD_MODE);

    qosa_task_sleep_ms(UNIR_IIC_INIT_DELAY);
    // Configure the codec register
    for (int i = 0; i < sizeof(reg_list) / sizeof(reg_list[0]); i++)
    {
        ret = qosa_i2c_write(QOSA_I2C_1, UNIR_ES8311_I2C_ADDR, reg_list[i].regAddr, (qosa_uint8_t *)&(reg_list[i].val), 1);
        qosa_task_sleep_ms(1);
    }
    return ret;
}

/**
 * @brief Voice call demo event callback function
 *
 * This function handles voice call-related events.
 *
 * @param[in] user_argv
 *          - User parameter, points to event ID
 *
 * @param[in] argv
 *          - Event data pointer, contains different structure data according to event type
 *
 * @return int
 *       - Returns 0 on success, -1 on failure
 */
static int voice_call_event_cb(void *user_argv, void *argv)
{
    qosa_notify_event_e      event_id = (qosa_ptr)user_argv;
    unir_ims_demo_ims_msg_t *msg = QOSA_NULL;

    msg = qosa_malloc(sizeof(unir_ims_demo_ims_msg_t));
    if (!msg)
    {
        QLOGV("msg alloc failed!");
        return -1;
    }

    msg->event_id = event_id;
    QLOGI("ims msg id:%d", msg->event_id);
    switch (event_id)
    {
        case QOSA_EVENT_MODEM_IMS_RING_STATUS:
        {
            QLOGI(">>> Incoming call RING, notifying main task to answer...");
            if (g_ring_sem != QOSA_NULL)
            {
                qosa_sem_release(g_ring_sem);   /* Wake up vc_demo_task */
            }
            break;
        }
        case QOSA_EVENT_MODEM_IMS_DISCONNECT_STATUS:
        {
            QLOGI(">>> Call hung up, NO CARRIER");
            break;
        }
        case QOSA_EVENT_MODEM_IMS_CONN_ID_EVNET:
        {
            qosa_ims_conn_id_event_t *conn = (qosa_ims_conn_id_event_t *)argv;
            if (conn)
            {
                QLOGI(">>> Call connected, remote number: %s", conn->dialnumstr);
            }
            break;
        }

        default:
            break;
    }

    qosa_free(msg);
    msg = QOSA_NULL;
    return 0;
}



/**
 * @brief Voice call demo task main function
 *
 * This function serves as the main loop of the voice call demo task, implementing the following:
 * - Register voice call demo event callback functions
 * - Check SIM card status and IMS registration status
 * - Wait for IMS attachment, retry on failure
 * - Periodically query and display IMS PDP IP information
 * - Support CFUN restart to restore voice call connection
 *
 * @param[in] arg
 *          - Task parameter
 */

static void vc_demo_task(void *arg)
{
    int          ret = 0;
    qosa_uint8_t simid = 0;
    int          wait_cnt = 0;
    int          max_wait = UNIR_IMS_REG_TIMEOUT_SEC / UNIR_IMS_REG_POLL_INTERVAL_SEC;
    char         phone_number[16] = {0};

    QOSA_UNUSED(arg);

    /* --- 1. Create RING semaphore --- */
    ret = qosa_sem_create(&g_ring_sem, 0);
    if (ret != QOSA_OK)
    {
        QLOGE("ring sem create failed");
        return;
    }

    /* --- 2. Initialize codec --- */
    ret = unir_codec_init();
    if (ret != 0)
    {
        QLOGE("codec init failed");
        return;
    }

    /* --- 3. Register RING / DISCONNECT / CONN events --- */
    qosa_event_notify_register(QOSA_EVENT_MODEM_IMS_RING_STATUS, voice_call_event_cb, (void *)QOSA_EVENT_MODEM_IMS_RING_STATUS);

    qosa_event_notify_register(QOSA_EVENT_MODEM_IMS_DISCONNECT_STATUS, voice_call_event_cb, (void *)QOSA_EVENT_MODEM_IMS_DISCONNECT_STATUS);

    qosa_event_notify_register(QOSA_EVENT_MODEM_IMS_CONN_ID_EVNET, voice_call_event_cb, (void *)QOSA_EVENT_MODEM_IMS_CONN_ID_EVNET);

    qosa_task_sleep_sec(5);
    /* --- 4. Wait for IMS registration (check PDP 15 IP via CGPADDR) --- */
    QLOGI("===== Waiting for IMS registration (PDP %d) ... =====", UNIR_IMS_PDPID);
    for (wait_cnt = 0; wait_cnt < max_wait; wait_cnt++)
    {
        qosa_datacall_ip_info_t info = {0};
        char                    ip_buf[64] = {0};

        ret = qosa_datacall_get_pdp_ip_info(simid, UNIR_IMS_PDPID, &info);
        if (ret == QOSA_DATACALL_OK && info.ip_type != QOSA_PDP_INVALID)
        {
            if (info.ip_type == QOSA_PDP_IPV4 || info.ip_type == QOSA_PDP_IPV4V6)
            {
                inet_ntop(QOSA_IP_ADDR_AF_INET, &info.ipv4_ip.addr.ipv4_addr, ip_buf, sizeof(ip_buf));
                QLOGI("+CGPADDR: %d,\"%s\"", UNIR_IMS_PDPID, ip_buf);
            }
            if (info.ip_type == QOSA_PDP_IPV6 || info.ip_type == QOSA_PDP_IPV4V6)
            {
                inet_ntop(QOSA_IP_ADDR_AF_INET6, &info.ipv6_ip.addr.ipv6_addr, ip_buf, sizeof(ip_buf));
                QLOGI("+CGPADDR: %d,\"%s\"", UNIR_IMS_PDPID, ip_buf);
            }
            QLOGI("===== IMS registration complete =====");
            break;
        }
        QLOGI("IMS PDP%d has no IP assigned, retrying in %d seconds...(%d/%d)",
              UNIR_IMS_PDPID, UNIR_IMS_REG_POLL_INTERVAL_SEC, wait_cnt + 1, max_wait);
        qosa_task_sleep_sec(UNIR_IMS_REG_POLL_INTERVAL_SEC);
    }

    if (wait_cnt >= max_wait)
    {
        QLOGE("===== IMS registration timeout (%d seconds) =====", UNIR_IMS_REG_TIMEOUT_SEC);
        return;
    }

    /* Wait a bit then start listening for incoming calls */
    qosa_task_sleep_sec(2);

    QLOGI("===== Demo ready, waiting for incoming RING ... =====");

    /* --- 5. Block waiting for RING event --- */
    ret = qosa_sem_wait(g_ring_sem, QOSA_WAIT_FOREVER);
    if (ret != QOSA_OK)
    {
        QLOGE("wait ring sem failed, ret=%d", ret);
        return;
    }

    QLOGI("===== RING received, executing ATA answer =====");
    qosa_task_sleep_sec(1);
    /* --- 6. Answer the call (ATA) --- */
    ret = qosa_answer_call();
    QLOGI("qosa_answer_call ret=%d", ret);

    /* Wait for call establishment */
    qosa_task_sleep_sec(2);

    /* --- 7. Hold the call for N seconds --- */
    QLOGI("===== Holding call for %d seconds, then hang up =====", UNIR_CALL_HOLD_SECONDS);
    qosa_task_sleep_sec(UNIR_CALL_HOLD_SECONDS);

    /* --- 8. Hang up the call (ATH) --- */
    QLOGI("===== Executing ATH hang up =====");
    ret = qosa_stop_voice_call(simid);
    QLOGI("qosa_stop_voice_call ret=%d", ret);

    /* Wait a bit then start dialing */
    qosa_task_sleep_sec(2);

    /* --- 9. Place an outgoing call --- */
    qosa_snprintf(phone_number, sizeof(phone_number), "18589831836");
    QLOGI("===== Dialing: %s =====", phone_number);
    ret = qosa_start_voice_call(phone_number);
    QLOGI("qosa_start_voice_call ret=%d", ret);

    /* Wait for call establishment */
    qosa_task_sleep_sec(5);

    /* Hold the call for N seconds */
    QLOGI("===== Holding call for %d seconds, then hang up =====", UNIR_OUTGOING_CALL_HOLD_SECONDS);
    qosa_task_sleep_sec(UNIR_OUTGOING_CALL_HOLD_SECONDS);

    /* Hang up */
    QLOGI("===== Executing ATH hang up =====");
    ret = qosa_stop_voice_call(simid);
    QLOGI("qosa_stop_voice_call ret=%d", ret);

    QLOGI("===== Demo finished =====");
}


/**
 * @brief Voice call demo initialization function
 *
 * This function initializes the voice call demo by creating the demo task.
 *
 */
void unir_voice_call_demo_init(void)
{
    int err = 0;
    // Create voice call demo task
    err = qosa_task_create(&g_vc_task, 1024 * 4, QOSA_PRIORITY_NORMAL, "QVCDEMO", vc_demo_task, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGD("voice call demo task create error");
        return;
    }

}
UNIRTOS_APP_EXPORT(329, "voice_call_demo", unir_voice_call_demo_init);
