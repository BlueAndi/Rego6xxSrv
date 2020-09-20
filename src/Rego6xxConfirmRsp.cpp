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
 * @brief  Rego6xx heatpump controller confirmation response
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxConfirmRsp.h"
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

bool Rego6xxConfirmRsp::isValid() const
{
    bool isValid = false;

    if (false == m_isPending)
    {
        isValid = true;
    }

    return isValid;
}

uint8_t Rego6xxConfirmRsp::getDevAddr() const
{
    uint8_t devAddr = 0;

    if ((false == isPending()) &&
        (true == isValid()))
    {
        devAddr = m_response[0];
    }

    return devAddr;
}

bool Rego6xxConfirmRsp::isConfirmed() const
{
    bool isConfirmed = false;

    if (false == m_isPending)
    {
        isConfirmed = true;
    }

    return isConfirmed;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Rego6xxConfirmRsp::receive()
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
