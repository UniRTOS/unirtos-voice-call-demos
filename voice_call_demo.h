/*****************************************************************/ /**
* @file voice_call_demo.h
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
#ifndef __VOICE_CALL_DEMO_H__
#define __VOICE_CALL_DEMO_H__

#include "qosa_def.h"
#include "qosa_sys.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define UNIR_ES8311_I2C_ADDR   0x18 /*!< 8311 codec slave address */
#define UNIR_IIC_SDA_NUM       66   /*!< IIC SDA pin number */
#define UNIR_IIC_SCL_NUM       67   /*!< IIC SCL pin number */
#define UNIR_IIC_FUNC          2    /*!< FUNC corresponding to IIC */
#define UNIR_IIC_INIT_DELAY    1000 /*!< Delay time after IIC initialization */
#define UNIR_IIC_REG_CFG_DELAY 1    /*!< Delay time after IIC register configuration */
#define UNIR_CODEC_8311_ID     0x83 /*!< ES8311 codec ID */
#define UNIR_IMS_PDPID                       15
#define UNIR_IMS_REG_TIMEOUT_SEC             60
#define UNIR_IMS_REG_POLL_INTERVAL_SEC       2
#define UNIR_CALL_HOLD_SECONDS               5
#define UNIR_OUTGOING_CALL_HOLD_SECONDS      8        /*!< Seconds to hold the call after dialing */

// es8311 codec register configuration
#define ES8311_INIT_REG                                                                                                                                        \
    {                                                                                                                                                          \
        {0x01, 0x30, 0x00}, {0x02, 0x00, 0x00}, {0x03, 0x20, 0x00}, {0x16, 0x20, 0x00}, {0x04, 0x20, 0x00}, {0x05, 0x00, 0x00}, {0x0B, 0x00, 0x00},            \
            {0x0C, 0x00, 0x00}, {0x0F, 0x44, 0x00}, {0x10, 0x1F, 0x00}, {0x11, 0x7F, 0x00}, {0x00, 0x80, 0x00}, {0x00, 0x80, 0x00}, {0x01, 0x3F, 0x00},        \
            {0x01, 0xBF, 0x00}, {0x02, 0x18, 0x00}, {0x05, 0x00, 0x00}, {0x03, 0x10, 0x00}, {0x04, 0x10, 0x00}, {0x07, 0x00, 0x00}, {0x08, 0xFF, 0x00},        \
            {0x06, 0x03, 0x00}, {0x01, 0xBF, 0x00}, {0x06, 0x03, 0x00}, {0x13, 0x10, 0x00}, {0x1B, 0x0A, 0x00}, {0x1C, 0x6A, 0x00}, {0x09, 0x0D, 0x00},        \
            {0x0A, 0x0D, 0x00}, {0x09, 0x0D, 0x00}, {0x0A, 0x0D, 0x00}, {0x32, 0xBF, 0x00}, {0x09, 0x0D, 0x00}, {0x0A, 0x0D, 0x00}, {0x17, 0xBF, 0x00},        \
            {0x0E, 0x02, 0x00}, {0x12, 0x00, 0x00}, {0x14, 0x1A, 0x00}, {0x14, 0x1A, 0x00}, {0x0D, 0x01, 0x00}, {0x15, 0x20, 0x00}, {0x37, 0x48, 0x00},        \
            {0x45, 0x00, 0x00}, {0x0D, 0x01, 0x00}, {0x45, 0x00, 0x00}, {0x37, 0x08, 0x00}, {0x14, 0x1A, 0x00}, {0x12, 0x00, 0x00}, {0x0E, 0x00, 0x00},        \
            {0x32, 0xBF, 0x00},                                                                                                                                \
    }
/*========================================================================
*  Enumeration Definition
*========================================================================*/

/*========================================================================
*  Structure Definition
*========================================================================*/
typedef struct
{
    qosa_uint32_t regAddr; /*!< codec register address */
    qosa_uint16_t val;     /*!< value of codec configuration */
    qosa_uint16_t delay;   /*!< codec configuration delay time */
} unir_codec_reg_t;
/*========================================================================
*  Func Definition
*========================================================================*/

void unir_voice_call_demo_init(void);

#endif /* __VOICE_CALL_DEMO_H__ */
