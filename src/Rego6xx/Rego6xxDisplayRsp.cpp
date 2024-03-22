/* MIT License
 *
 * Copyright (c) 2020 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Rego6xx heatpump controller display response
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxDisplayRsp.h"
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

bool Rego6xxDisplayRsp::isValid() const
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

uint8_t Rego6xxDisplayRsp::getDevAddr() const
{
    uint8_t devAddr = 0;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        devAddr = m_response[0];
    }

    return devAddr;
}

String Rego6xxDisplayRsp::getMsg() const
{
    String  text;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        const uint8_t   MAX_LEN         = 40;
        const uint8_t   TEXT_START_IDX  = 1;
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
            char    uChar       = static_cast<char>(character);

            if ('\0' != uChar)
            {
                text.concat(uChar);
            }

            idx += 2;
        }
    }

    return text;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
