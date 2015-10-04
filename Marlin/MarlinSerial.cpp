/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include "Marlin.h"
#include "MarlinSerial.h"

#ifndef USBCON
// this next line disables the entire HardwareSerial.cpp,
// this is so I can support Attiny series and any other chip without a UART
#if defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(UBRR2H) || defined(UBRR3H)

#if UART_PRESENT(0)
  #if defined(M_USARTx_RX_vect)
    SIGNAL(M_USARTx_RX_vect(0)) {
      unsigned char c  =  M_UDRx(0);
      customizedSerial.rx_buffer.store_char(c);
    }
  #endif
#endif

#if UART_PRESENT(1)
  #if defined(M_USARTx_RX_vect)
    SIGNAL(M_USARTx_RX_vect(1)) {
      unsigned char c  =  M_UDRx(1);
      customizedSerial1.rx_buffer.store_char(c);
    }
  #endif
#endif

#if UART_PRESENT(2)
  #if defined(M_USARTx_RX_vect)
    SIGNAL(M_USARTx_RX_vect(2)) {
      unsigned char c  =  M_UDRx(2);
      customizedSerial2.rx_buffer.store_char(c);
    }
  #endif
#endif

#if UART_PRESENT(3)
  #if defined(M_USARTx_RX_vect)
    SIGNAL(M_USARTx_RX_vect(3)) {
      unsigned char c  =  M_UDRx(3);
      customizedSerial3.rx_buffer.store_char(c);
    }
  #endif
#endif

// Public Methods //////////////////////////////////////////////////////////////

void MarlinSerial::begin(long baud) {
  uint16_t baud_setting;
  bool useU2X = true;

  #if F_CPU == 16000000UL && SERIAL_PORT == 0
    // hard-coded exception for compatibility with the bootloader shipped
    // with the Duemilanove and previous boards and the firmware on the 8U2
    // on the Uno and Mega 2560.
    if (baud == 57600) {
      useU2X = false;
    }
  #endif

  #define BEGIN_PORT(p) \
    case p: \
      if (useU2X) { \
        M_UCSRxA( p ) = BIT(M_U2Xx( p )); \
        baud_setting = (F_CPU / 4 / baud - 1) / 2; \
      } else { \
        M_UCSRxA( p ) = 0; \
        baud_setting = (F_CPU / 8 / baud - 1) / 2; \
      } \
      M_UBRRxH( p ) = baud_setting >> 8; \
      M_UBRRxL( p ) = baud_setting; \
      sbi(M_UCSRxB( p ), M_RXENx( p )); \
      sbi(M_UCSRxB( p ), M_TXENx( p )); \
      sbi(M_UCSRxB( p ), M_RXCIEx( p )); \
      break;

  switch(port) {
    #if UART_PRESENT(0)
      BEGIN_PORT(0)
    #endif
    #if UART_PRESENT(1)
      BEGIN_PORT(1)
    #endif
    #if UART_PRESENT(2)
      BEGIN_PORT(2)
    #endif
    #if UART_PRESENT(3)
      BEGIN_PORT(3)
    #endif
  }
}

void MarlinSerial::end() {
  #define END_PORT(p) \
    case p: \
      cbi(M_UCSRxB(p), M_RXENx(p)); \
      cbi(M_UCSRxB(p), M_TXENx(p)); \
      cbi(M_UCSRxB(p), M_RXCIEx(p)); \
      break;
  switch(port) {
    #if UART_PRESENT(0)
      END_PORT(0)
    #endif
    #if UART_PRESENT(1)
      END_PORT(1)
    #endif
    #if UART_PRESENT(2)
      END_PORT(2)
    #endif
    #if UART_PRESENT(3)
      END_PORT(3)
    #endif
  }
}


int MarlinSerial::peek(void) {
  if (rx_buffer.head == rx_buffer.tail) {
    return -1;
  }
  else {
    return rx_buffer.buffer[rx_buffer.tail];
  }
}

int MarlinSerial::read(void) {
  // if the head isn't ahead of the tail, we don't have any characters
  if (rx_buffer.head == rx_buffer.tail) {
    return -1;
  }
  else {
    unsigned char c = rx_buffer.buffer[rx_buffer.tail];
    rx_buffer.tail = (unsigned int)(rx_buffer.tail + 1) % RX_BUFFER_SIZE;
    return c;
  }
}

void MarlinSerial::flush() {
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of rx_buffer_head but before writing
  // the value to rx_buffer_tail; the previous value of rx_buffer_head
  // may be written to rx_buffer_tail, making it appear as if the buffer
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of rx_buffer_head but before writing
  // the value to rx_buffer_tail; the previous value of rx_buffer_head
  // may be written to rx_buffer_tail, making it appear as if the buffer
  // were full, not empty.
  rx_buffer.head = rx_buffer.tail;
}


/// imports from print.h


void MarlinSerial::print(char c, int base) {
  print((long) c, base);
}

void MarlinSerial::print(unsigned char b, int base) {
  print((unsigned long) b, base);
}

void MarlinSerial::print(int n, int base) {
  print((long) n, base);
}

void MarlinSerial::print(unsigned int n, int base) {
  print((unsigned long) n, base);
}

void MarlinSerial::print(long n, int base) {
  if (base == 0) {
    write(n);
  }
  else if (base == 10) {
    if (n < 0) {
      print('-');
      n = -n;
    }
    printNumber(n, 10);
  }
  else {
    printNumber(n, base);
  }
}

void MarlinSerial::print(unsigned long n, int base) {
  if (base == 0) write(n);
  else printNumber(n, base);
}

void MarlinSerial::print(double n, int digits) {
  printFloat(n, digits);
}

void MarlinSerial::println(void) {
  print('\r');
  print('\n');
}

void MarlinSerial::println(const String& s) {
  print(s);
  println();
}

void MarlinSerial::println(const char c[]) {
  print(c);
  println();
}

void MarlinSerial::println(char c, int base) {
  print(c, base);
  println();
}

void MarlinSerial::println(unsigned char b, int base) {
  print(b, base);
  println();
}

void MarlinSerial::println(int n, int base) {
  print(n, base);
  println();
}

void MarlinSerial::println(unsigned int n, int base) {
  print(n, base);
  println();
}

void MarlinSerial::println(long n, int base) {
  print(n, base);
  println();
}

void MarlinSerial::println(unsigned long n, int base) {
  print(n, base);
  println();
}

void MarlinSerial::println(double n, int digits) {
  print(n, digits);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void MarlinSerial::printNumber(unsigned long n, uint8_t base) {
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
  unsigned long i = 0;

  if (n == 0) {
    print('0');
    return;
  }

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    print((char)(buf[i - 1] < 10 ?
                 '0' + buf[i - 1] :
                 'A' + buf[i - 1] - 10));
}

void MarlinSerial::printFloat(double number, uint8_t digits) {
  // Handle negative numbers
  if (number < 0.0) {
    print('-');
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) print('.');

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= 10.0;
    int toPrint = int(remainder);
    print(toPrint);
    remainder -= toPrint;
  }
}
// Preinstantiate Objects //////////////////////////////////////////////////////


#if UART_PRESENT(0)
  MarlinSerial customizedSerial(0);
#endif

#if UART_PRESENT(1)
  MarlinSerial customizedSerial1(1);
#endif

#if UART_PRESENT(2)
  MarlinSerial customizedSerial2(2);
#endif

#if UART_PRESENT(3)
  MarlinSerial customizedSerial3(3);
#endif

#endif // whole file
#endif // !USBCON

// For AT90USB targets use the UART for BT interfacing
#if defined(USBCON) && ENABLED(BLUETOOTH)
  HardwareSerial bluetoothSerial;
#endif
