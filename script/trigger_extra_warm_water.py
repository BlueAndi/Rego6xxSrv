"""
MIT License

Copyright (c) 2020 - 2024 Andreas Merkle <web@blue-andi.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

================================================================================
"""

import sys
import requests

HEATPUMP_URL = 'http://heatpump.fritz.box'

char_map = {
    0xfffd: '?' # Map unknown UTF-8 character to '?'
}

ROOT_MENU_ROW_4 = 'W?rme   Info    Men?'
BACK_BUTTON_ROW_4 = 'Zur?ck'
CANCEL_BUTTON_ROW_4 = 'Abbr.'

action_list = [{
    'row': 4,
    'expected': 'W?rme   Info    Men?',
    'action': 'buttonR'
}, {
    'row': 2,
    'expected': 'Hausw?rme',
    'action': 'wheelTR'
}, {
    'row': 2,
    'expected': 'Warmwassereinst.',
    'action': 'buttonR'
}, {
    'row': 3,
    'expected': 'Extra Warmwasser',
    'action': 'buttonR'
}, {
    'row': 1,
    'expected': '  Extra Warmwasser',
    'action': 'buttonR'
}, {
    'row': 1,
    'expected': '  Extra Warmwasser',
    'action': 'wheelTR'
}, {
    'row': 4,
    'expected': 'Abbr.      Speichern',
    'action': 'buttonR'
}]

def get_display_content(base_uri, row):
    """Get display row content.

    Args:
        base_uri (str): The base URI of the server.
        row (int): Row number [1; 4]

    Returns:
        str: The row content as string.
    """
    row_min = 1
    row_max = 4
    content = ''

    if row_min <= row <= row_max:
        dst_url = base_uri + '/api/display/' + str(row)
        response = requests.get(dst_url, headers={"Content-Type": "application/json"})

        if response.status_code == 200:
            # The response may contain control characters, therefore disable strict parsing.
            json_doc = response.json(strict=False)
            # The response may contain non UTF-8 conform characters, replace them via '?'.
            content = json_doc['data']['display'].translate(char_map)

    return content

def show_display(base_uri):
    """Show the whole display content on the console.

    Args:
        base_uri (str): The base URI of the server.
    """
    row_min = 1
    row_max = 4

    print("--------------------")
    for row in range(row_min, (row_max + 1)):
        print(get_display_content(base_uri, row))
    print("--------------------")

def manipulate_keyboard(base_uri, hmi_device):
    """Manipulate the HMI interface of the heatpump.

    Args:
        base_uri (str): The base URI of the server.
        hmi_device (str): The HMI specific name, see REST API.

    Returns:
        boolean: If successful it will return True otherwise False.
    """
    is_successful = False
    dst_url = base_uri + '/api/frontPanel/' + hmi_device
    response = requests.post(dst_url, headers={"Content-Type": "application/json"})

    if response.status_code == 200:
        is_successful = True

    return is_successful

def press_left_button(base_uri):
    """Press the left button of the HMI.

    Args:
        base_uri (str): The base URI of the server.

    Returns:
        boolean: If successful it will return True otherwise False.
    """
    return manipulate_keyboard(base_uri, 'buttonL')

def press_right_button(base_uri):
    """Press the right button of the HMI.

    Args:
        base_uri (str): The base URI of the server.

    Returns:
        boolean: If successful it will return True otherwise False.
    """
    return manipulate_keyboard(base_uri, 'buttonR')

def turn_wheel_left(base_uri):
    """Turn the wheel left of the HMI.

    Args:
        base_uri (str): The base URI of the server.

    Returns:
        boolean: If successful it will return True otherwise False.
    """
    return manipulate_keyboard(base_uri, 'wheelTL')

def turn_wheel_right(base_uri):
    """Turn the wheel right of the HMI.

    Args:
        base_uri (str): The base URI of the server.

    Returns:
        boolean: If successful it will return True otherwise False.
    """
    return manipulate_keyboard(base_uri, 'wheelTR')

if __name__ == '__main__':

    RET_VAL = 0 # Success

    # The menu shall be in the root menu
    row_4 = get_display_content(HEATPUMP_URL, 4)
    while row_4 != ROOT_MENU_ROW_4:
        if row_4.startswith(BACK_BUTTON_ROW_4) or row_4.startswith(CANCEL_BUTTON_ROW_4):
            if press_left_button(HEATPUMP_URL) is False:
                RET_VAL = 1 # Error
                break
            else:
                row_4 = get_display_content(HEATPUMP_URL, 4)
        else:
            RET_VAL = 2 # Error
            break

    if RET_VAL == 0:
        # Execute the actions in the action list to trigger
        # extra warm water.
        for action in action_list:

            # Get expected string on the display from the specified row.
            # This will be used to check whether we are in the correct menu
            # and to avoid configuring something bad.
            row_content = get_display_content(HEATPUMP_URL, action['row'])

            # Does the expected string on the display not match?
            if row_content.startswith(action['expected']) is False:
                show_display(HEATPUMP_URL)
                print('Expected: "' + action['expected'] + '"')
                RET_VAL = 3 # Error
                break
            else:
                # We are in the correct menu, execute action on HMI.
                if manipulate_keyboard(HEATPUMP_URL, action['action']) is False:
                    print('Invalid action.')
                    RET_VAL = 4 # Error
                    break

    #show_display(HEATPUMP_URL)

    sys.exit(RET_VAL)
