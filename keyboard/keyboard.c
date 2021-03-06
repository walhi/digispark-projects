/**
 * Project: AVR ATtiny USB Tutorial at http://codeandlife.com/
 * Author: Joonas Pihlajamaa, joonas.pihlajamaa@iki.fi
 * Fork: Sergey V. Karpesh, walhi@walhi.ru
 * Base on V-USB example code by Christian Starkjohann
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v3 (see License.txt)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "usbdrv.h"
#include "usb_hid_keys.h"
#include "signals.h"

// ************************
// *** USB HID ROUTINES ***
// ************************

// From Frank Zhao's USB Business Card project
// http://www.frank-zhao.com/cache/usbbusinesscard_details.php
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x06, // USAGE (Keyboard)
	0xa1, 0x01, // COLLECTION (Application)
	0x75, 0x01, //   REPORT_SIZE (1)
	0x95, 0x08, //   REPORT_COUNT (8)
	0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x01, //   LOGICAL_MAXIMUM (1)
	0x81, 0x02, //   INPUT (Data,Var,Abs) ; Modifier byte
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x08, //   REPORT_SIZE (8)
	0x81, 0x03, //   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05, //   REPORT_COUNT (5)
	0x75, 0x01, //   REPORT_SIZE (1)
	0x05, 0x08, //   USAGE_PAGE (LEDs)
	0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05, //   USAGE_MAXIMUM (Kana)
	0x91, 0x02, //   OUTPUT (Data,Var,Abs) ; LED report
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x03, //   REPORT_SIZE (3)
	0x91, 0x03, //   OUTPUT (Cnst,Var,Abs) ; LED report padding
	0x95, 0x06, //   REPORT_COUNT (6)
	0x75, 0x08, //   REPORT_SIZE (8)
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x65, //   LOGICAL_MAXIMUM (101)
	0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)(101)
	0x81, 0x00, //   INPUT (Data,Ary,Abs)
	0xc0        // END_COLLECTION
};

typedef struct {
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycode[6];
} keyboard_report_t;

static keyboard_report_t keyboard_report __attribute__ ((section (".noinit"))); // sent to PC
volatile static uchar LED_state = 0xff; // received from PC
static uchar state_caps = 0x0;
static uchar count_caps = 0x0; // for reboot
static uchar idleRate; // repeat rate for keyboards


usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
		switch (rq->bRequest) {
		case USBRQ_HID_GET_REPORT: // send "no keys pressed" if asked here
			// wValue: ReportType (highbyte), ReportID (lowbyte)
			usbMsgPtr = (void *)&keyboard_report; // we only have this one
			keyboard_report.modifier = 0;
			keyboard_report.keycode[0] = 0;
			return sizeof(keyboard_report);
		case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
			return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
		case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
			usbMsgPtr = &idleRate;
			return 1;
		case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
			idleRate = rq->wValue.bytes[1];
			return 0;
		}
	}

	return 0; // by default don't return any data
}

#define NUM_LOCK 1
#define CAPS_LOCK 2
#define SCROLL_LOCK 4

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len) {
	if (data[0] == LED_state){
		return 1;
	} else {
		LED_state = data[0];
	}

	state_caps = LED_state & CAPS_LOCK;
	if (state_caps){
		if ((PORTB & _BV(PB1)) == 0) count_caps++;
		PORTB |= _BV(PB1);
	} else {
		PORTB &= ~_BV(PB1);
	}

	return 1;
}

// For keycodes only
void buildReport(uchar keycode, uchar modifier) {
	keyboard_report.modifier = modifier;
	keyboard_report.keycode[0] = keycode;
}

// Only letters and 0 (NULL) to clear buttons. The function displays the character in the desired register, regardless of the status caps_lock
void buildReportChar(uchar letter) {
	uchar modifier = KEY_NONE;
	if (letter >= 'A'){
		// Буквы
		if (letter >= 'A' && letter <= 'Z'){
			if (!state_caps) modifier = KEY_MOD_LSHIFT;
			letter -= 'A' - 'a';
		} else {
			if (state_caps) modifier = KEY_MOD_LSHIFT;
		}

		if (letter >= 'a' && letter <= 'z'){
			keyboard_report.keycode[0] = 4 + (letter - 'a');
		} else {
			keyboard_report.keycode[0] = 0;
		}
	} else if (letter >= '0' && letter <= '9'){
		// Цифры
		if (letter == '0'){
			keyboard_report.keycode[0] = 0x27;
		} else {
			keyboard_report.keycode[0] = 0x1d + (letter - '0');
		}
	} else {
		// символы
		modifier = KEY_MOD_LSHIFT;
		switch(letter){
		case '!':
			keyboard_report.keycode[0] = 0x1d + 1;
			break;
		case '@':
			keyboard_report.keycode[0] = 0x1d + 2;
			break;
		case '#':
			keyboard_report.keycode[0] = 0x1d + 3;
			break;
		case '$':
			keyboard_report.keycode[0] = 0x1d + 4;
			break;
		case '%':
			keyboard_report.keycode[0] = 0x1d + 5;
			break;
		case '^':
			keyboard_report.keycode[0] = 0x1d + 6;
			break;
		case '&':
			keyboard_report.keycode[0] = 0x1d + 7;
			break;
		case '*':
			keyboard_report.keycode[0] = 0x1d + 8;
			break;
		case '(':
			keyboard_report.keycode[0] = 0x1d + 9;
			break;
		case ')':
			keyboard_report.keycode[0] = 0x27;
			break;
		case ':':
			keyboard_report.keycode[0] = 0x33;
			break;
		case ';':
			modifier = KEY_NONE;
			keyboard_report.keycode[0] = 0x33;
			break;
		}
	}
	keyboard_report.modifier = modifier;
}

void buildReportChar2(uchar letter) {
	uchar modifier = KEY_NONE;

}


#define STATE_WAIT 0
#define STATE_SEND_KEY 1
#define STATE_SEND_RELEASE 2
#define STATE_RELEASE_WAIT 3

char msg[] = "keyboard test";

int main() {
	uchar i;
	uchar state = STATE_WAIT;
	char* ptr = msg;

	for(i=0; i<sizeof(keyboard_report); i++) ((uchar *)&keyboard_report)[i] = 0;

	wdt_enable(WDTO_1S); // enable 1s watchdog timer

	KEY_PORT_REG |= KEY_PIN; // pull up
	LED_DDR_REG |= LED_PIN;

	usbInit();

	usbDeviceDisconnect(); // enforce re-enumeration
	wdt_reset();
	_delay_ms(500);
	usbDeviceConnect();

	sei(); // Enable interrupts after re-enumeration

	while (1) {
		wdt_reset();
		usbPoll();

		if (count_caps >= 5) do {} while (1);

		if (usbInterruptIsReady() && LED_state != 0xff) {
			switch(state) {
			case STATE_SEND_KEY:
				if (*ptr != 0){
					buildReportChar(*ptr);
				}
				state = STATE_SEND_RELEASE;
				break;
			case STATE_SEND_RELEASE:
				buildReport(KEY_NONE, KEY_NONE);
				ptr++;
				if (*ptr == 0){
					ptr = msg;
					state = STATE_RELEASE_WAIT;
				} else {
					state = STATE_SEND_KEY;
				}
				break;
			case STATE_RELEASE_WAIT:
				if (KEY_PIN_REG & KEY_PIN){
					state = STATE_WAIT;
				} /*else {0koT60
					continue;
					}*/
				break;
			default:
				state = STATE_WAIT;
				if ((KEY_PIN_REG & KEY_PIN) == 0){
					count_caps = 0; // clear reboot counter
					state = STATE_SEND_KEY;
				} else {
					continue;
				}
			}
			usbSetInterrupt((void *)&keyboard_report, sizeof(keyboard_report));
		}
	}
	return 0;
}
