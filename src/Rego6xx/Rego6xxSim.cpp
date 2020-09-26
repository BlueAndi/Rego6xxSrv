/* MIT License
 *
 * Copyright (c) 2020 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Rego6xx heatpump controller simulator
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxSim.h"
#include "Rego6xxUtil.h"
#include "Rego6xxCtrl.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

int Rego6xxSim::read()
{
    int result = -1;

    if (m_rspSize > m_readIndex)
    {
        Serial.printf("Rx: %02X\n", m_rspBuffer[m_readIndex]);

        result = m_rspBuffer[m_readIndex];
        ++m_readIndex;
    }

    return result;
}

int Rego6xxSim::peek()
{
    int result = -1;

    if (m_rspSize > m_readIndex)
    {
        result = m_rspBuffer[m_readIndex];
    }

    return result;
}

size_t Rego6xxSim::write(const uint8_t* buffer, size_t size)
{
    size_t index = 0;

    Serial.printf("Tx: ");

    while(size > index)
    {
        Serial.printf("%02X", buffer[index]);
        ++index;
    }

    Serial.printf("\n");

    /* Prepare response */
    m_readIndex = 0;
    prepareRsp(buffer, size);

    return size;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Rego6xxSim::generateStdRsp(uint16_t value)
{
    m_rspSize = 5;

    m_rspBuffer[0] = Rego6xxCtrl::DEV_ADDR_HOST;
    m_rspBuffer[1] = (value >> 14) & 0x03;
    m_rspBuffer[2] = (value >>  7) & 0x7f;
    m_rspBuffer[3] = (value >>  0) & 0x7f;
    m_rspBuffer[4] = Rego6xxUtil::calculateChecksum(&m_rspBuffer[1], m_rspSize - 2);
}

void Rego6xxSim::generateConfirmRsp()
{
    m_rspSize = 1;

    m_rspBuffer[0] = Rego6xxCtrl::DEV_ADDR_HOST;
}

void Rego6xxSim::generateTextRsp(const String& text)
{
    const uint8_t   MAX_LEN = 20;
    uint8_t         idx     = 0;

    m_rspSize = 42;

    m_rspBuffer[0] = Rego6xxCtrl::DEV_ADDR_HOST;

    while((MAX_LEN > idx) && (text.length() > idx))
    {
        m_rspBuffer[idx + 1] = (static_cast<uint8_t>(text[idx]) & 0xf0) >> 4;
        m_rspBuffer[idx + 2] = (static_cast<uint8_t>(text[idx]) & 0x0f) >> 0;
        idx += 2;
    }

    m_rspBuffer[4] = Rego6xxUtil::calculateChecksum(&m_rspBuffer[1], m_rspSize - 2);
}

void Rego6xxSim::generateErrprRsp()
{
    const uint8_t   data[]  =
    {
        0x01, 0x06, 0x03, 0x00, 0x03, 0x02, 0x03, 0x01,
        0x03, 0x00, 0x03, 0x00, 0x03, 0x09, 0x02, 0x00,
        0x03, 0x01, 0x03, 0x08, 0x03, 0x0A, 0x03, 0x02,
        0x03, 0x01, 0x03, 0x0A, 0x03, 0x00, 0x03, 0x03,
        0x00, 0x00, 0x00, 0x01, 0x04, 0x06, 0x00, 0x02
    };
    uint8_t         idx     = 0;

    m_rspSize = 42;

    m_rspBuffer[0] = Rego6xxCtrl::DEV_ADDR_HOST;

    while((sizeof(data) / sizeof(data[1])) > idx)
    {
        m_rspBuffer[idx + 1] = data[idx];
        ++idx;
    }

    m_rspBuffer[41] = Rego6xxUtil::calculateChecksum(&m_rspBuffer[1], m_rspSize - 2);
}

void Rego6xxSim::generateBoolRsp(bool value)
{
    uint16_t u16Value = (false == value) ? 0 : 1;

    m_rspSize = 5;

    m_rspBuffer[0] = Rego6xxCtrl::DEV_ADDR_HOST;
    m_rspBuffer[1] = (u16Value >> 14) & 0x03;
    m_rspBuffer[2] = (u16Value >>  7) & 0x7f;
    m_rspBuffer[3] = (u16Value >>  0) & 0x7f;
    m_rspBuffer[4] = Rego6xxUtil::calculateChecksum(&m_rspBuffer[1], m_rspSize - 2);
}

void Rego6xxSim::prepareRsp(const uint8_t* buffer, size_t size)
{
    if (Rego6xxCtrl::CMD_SIZE != size)
    {
        generateStdRsp(0);
    }
    else
    {
        switch(buffer[1])
        {
        case Rego6xxCtrl::CMD_ID_READ_FRONT_PANEL:
            {
                uint16_t    addr;

                addr  = ((uint16_t)(buffer[2] & 0x03)) << 14;
                addr |= ((uint16_t)(buffer[3] & 0x7f)) <<  7;
                addr |= ((uint16_t)(buffer[4] & 0x7f)) <<  0;

                Serial.printf("Read front panel addr 0x%04X.\n", addr);

                generateBoolRsp(true);
            }
            break;

        case Rego6xxCtrl::CMD_ID_WRITE_FRONT_PANEL:
            generateConfirmRsp(); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_READ_SYSTEM_REG:
            {
                uint16_t    addr;

                addr  = ((uint16_t)(buffer[2] & 0x03)) << 14;
                addr |= ((uint16_t)(buffer[3] & 0x7f)) <<  7;
                addr |= ((uint16_t)(buffer[4] & 0x7f)) <<  0;

                Serial.printf("Read system register 0x%04X.\n", addr);

                generateStdRsp(240);    /* 24.0 °C ... just a value */
            }
            break;

        case Rego6xxCtrl::CMD_ID_WRITE_SYSTEM_REG:
            {
                uint16_t    addr;
                uint16_t    value;

                addr  = ((uint16_t)(buffer[2] & 0x03)) << 14;
                addr |= ((uint16_t)(buffer[3] & 0x7f)) <<  7;
                addr |= ((uint16_t)(buffer[4] & 0x7f)) <<  0;

                value  = ((uint16_t)(buffer[5] & 0x03)) << 14;
                value |= ((uint16_t)(buffer[6] & 0x7f)) <<  7;
                value |= ((uint16_t)(buffer[7] & 0x7f)) <<  0;

                Serial.printf("Write %u to system register 0x%04X.\n", value, addr);

                generateConfirmRsp();
            }
            break;

        case Rego6xxCtrl::CMD_ID_READ_TIMER_REG:
            generateStdRsp(0); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_WRITE_TIMER_REG:
            generateConfirmRsp(); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_READ_REG_1B61:
            generateStdRsp(0); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_WRITE_REG_1B61:
            generateConfirmRsp(); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_READ_DISPLAY:
            generateTextRsp(""); /* Not supported yet. */
            break;

        case Rego6xxCtrl::CMD_ID_READ_LAST_ERROR:
            generateErrprRsp();
            break;

        case Rego6xxCtrl::CMD_ID_READ_PREV_ERROR:
            generateErrprRsp();
            break;

        case Rego6xxCtrl::CMD_ID_READ_REGO_VERSION:
            Serial.printf("Read Rego6xxx version.\n");

            generateStdRsp(0x0258); /* 0x0258 for Rego600 */
            break;

        default:
            generateStdRsp(0); /* Unknown command */
            break;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
