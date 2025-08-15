#ifndef _PORTIO_H
#define _PORTIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** out:
 *  Sends the given data to the given I/O port
 *
 *  @param port The I/O port to send the data to
 *  @param data The data to send to the I/O port
 */
void out(uint16_t port, uint8_t data);

/** in:
 *  Read data from the given I/O port
 *
 *  @param port The I/O port to request the data
 *  @return Recieved data from the corresponding I/O port
 */
uint8_t in(uint16_t port);

//I/O port 16-bit for sending data from the disk controller
void out16(uint16_t port, uint16_t data);

//I/O port 16-bit for receiving to the disk controller
uint16_t in16(uint16_t port);

#endif