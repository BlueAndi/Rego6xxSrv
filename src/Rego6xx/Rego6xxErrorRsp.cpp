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
 * @brief  Rego6xx heatpump controller error response
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxErrorRsp.h"
#include "Rego6xxUtil.h"

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

bool Rego6xxErrorRsp::isValid() const
{
    bool isValid = false;

    if (false == isPending())
    {
        if (m_response[RSP_SIZE - 1] == Rego6xxUtil::calculateChecksum(&m_response[1], RSP_SIZE - 2))
        {
            isValid = true;
        }
    }

    return isValid;
}

uint8_t Rego6xxErrorRsp::getDevAddr() const
{
    uint8_t devAddr = 0;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        devAddr = m_response[0];
    }

    return devAddr;
}

uint8_t Rego6xxErrorRsp::getErrorId() const
{
    uint8_t errorId = 0;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        const uint8_t   ERROR_ID_START_IDX  = 1;
        uint8_t         column              = m_response[ERROR_ID_START_IDX + 0] & 0x0f;
        uint8_t         row                 = m_response[ERROR_ID_START_IDX + 1] & 0x0f;
        
        errorId = (column << 4) | (row << 0);
    }

    return errorId;
}

String Rego6xxErrorRsp::getErrorLog() const
{
    String  text;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        const uint8_t   MAX_LEN         = 30;
        const uint8_t   TEXT_START_IDX  = 3;
        uint8_t         idx             = TEXT_START_IDX;

        /* Characters are coded as four bit pairs. First character informing
         * about column, second about row of character. For standard
         * characters is encoding same as computer character table, in that
         * case is possible to concat doubles and present it directly.
         */
        while((MAX_LEN + TEXT_START_IDX) > idx)
        {
            uint8_t column      = m_response[idx + 0] & 0x0f;
            uint8_t row         = m_response[idx + 1] & 0x0f;
            uint8_t character   = (column << 4) | (row << 0);

            text.concat(static_cast<char>(character));

            idx += 2;
        }
    }

    return text;
}

String Rego6xxErrorRsp::getErrorDescription() const
{
    String description;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        uint8_t errorId = getErrorId();

        switch(errorId)
        {
        case 0:
            description = F("Sensor radiator return (GT1)");
            break;

        case 1:
            description = F("Outdoor sensor (GT2)");
            break;

        case 2:
            description = F("Sensor hot water (GT3)");
            break;

        case 3:
            description = F("Mixing valve sensor (GT4)");
            break;

        case 4:
            description = F("Room sensor (GT5)");
            break;

        case 5:
            description = F("Sensor compressor (GT6)");
            break;

        case 6:
            description = F("Sensor heat tran fluid out (GT8)");
            break;
        
        case 7:
            description = F("Sensor heat tran fluid in (GT9)");
            break;

        case 8:
            description = F("Sensor cold tran fluid in (GT10)");
            break;

        case 9:
            description = F("Sensor cold tran fluid in (GT11)");
            break;

        case 10:
            description = F("Compresor circuit switch");
            break;

        case 11:
            description = F("Electrical cassette");
            break;

        case 12:
            description = F("HTF C=pump switch (MB2)");
            break;

        case 13:
            description = F("Low pressure switch (LP)");
            break;

        case 14:
            description = F("High pressure switch (HP)");
            break;

        case 15:
            description = F("High return HP (GT9)");
            break;

        case 16:
            description = F("HTF out max (GT8)");
            break;

        case 17:
            description = F("HTF in under limit (GT10)");
            break;

        case 18:
            description = F("HTF out under limit (GT11)");
            break;

        case 19:
            description = F("Compressor superhear (GT6)");
            break;

        case 20:
            description = F("3-phase incorrect order");
            break;

        case 21:
            description = F("Power failure");
            break;

        case 22:
            description = F("Varmetr. delta high");
            break;

        default:
            description = F("?");
            break;
        }
    }

    return description;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Rego6xxErrorRsp::receive()
{
    /* Response pending? */
    if (true == m_isPending)
    {
        /* Start response timeout observation as soon as possible. */
        if (false == m_timer.isTimerRunning())
        {
            m_timer.start(TIMEOUT);
        }
        /* Timeout? */
        else if (true == m_timer.isTimeout())
        {
            m_stream.flush();
            m_isPending = false;
            memset(m_response, 0, sizeof(m_response));
            m_timer.stop();
        }
        /* Response complete received? */
        else if (RSP_SIZE <= m_stream.available())
        {
            uint8_t idx = 0;

            while(RSP_SIZE > idx)
            {
                m_response[idx] = m_stream.read();
                ++idx;
            }

            m_isPending = false;
            m_timer.stop();
        }
        /* Waiting for response. */
        else
        {
            /* Nothing to do. */
            ;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
