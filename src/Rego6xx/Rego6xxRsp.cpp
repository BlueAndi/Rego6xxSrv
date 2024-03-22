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
 * @brief  Rego6xx heatpump controller response base
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxRsp.h"

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

void Rego6xxRsp::receive()
{
    /* Response pending? */
    if (true == m_isPending)
    {
        uint8_t*    buffer  = nullptr;
        size_t      size    = 0;

        getResponse(buffer, size);

        /* Buffer for response available? */
        if ((nullptr == buffer) ||
            (0 == size))
        {
            m_stream.flush();
            m_isPending = false;
            m_timer.stop();
        }
        /* Start response timeout observation as soon as possible. */
        else if (false == m_timer.isTimerRunning())
        {
            m_timer.start(TIMEOUT);
        }
        /* Timeout? */
        else if (true == m_timer.isTimeout())
        {
            m_stream.flush();
            m_isPending = false;
            memset(buffer, 0, size);
            m_timer.stop();
        }
        /* Response complete received? */
        else if (static_cast<int>(size) <= m_stream.available())
        {
            uint8_t idx = 0;

            while(size > idx)
            {
                buffer[idx] = m_stream.read();
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
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
