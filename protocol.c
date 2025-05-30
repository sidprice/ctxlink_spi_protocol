/**
 * @file protocol.cpp
 * @author Sid Price (sid@sidprice.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-02
 * 
 * @copyright Copyright Sid Price (c) 2025
 * 
 * This file contains the implementation of the protocol used for communications between the ESP32
 * and ctxLink.
 */

#include "protocol.h"

#define PACKET_HEADER_SIZE PACKET_HEADER_DATA_START; // Size of the header before the data starts

/**
 * @brief Package the passed data into a buffer for transmission
 * 
 * @param buffer            Pointer to the input/output buffer
 * @param data_length       Length of the data to be packaged
 * @param buffer_size       SIze of the buffer
 * 
 * Protocol for the packaged data:
 *      Byte 0: First byte of 'magic number'
 *      Byte 1: Second byte of 'magic number'
 *      Byte 2: Message source identifier (e.g. 0x01 for GDB)
 *      Byte 3: MS Byte of the byte count
 *      Byte 4: LS Byte of the byte count
 *      Byte 5: First data byte
 *          ...
 *      Byte (n-1)+5    : Last data byte
 * 
 */
size_t package_data(uint8_t *buffer, size_t data_length, protocol_packet_type_e data_type)
{
	memmove(buffer + PACKET_HEADER_DATA_START, buffer, data_length); // Move the data to start 3 bytes in

	size_t packet_length = data_length + PACKET_HEADER_SIZE; // Total packet length
	size_t padding = packet_length % 4;                      // Calculate padding to make the total size a multiple of 4
	buffer[PACKET_HEADER_MAGIC1] = PROTOCOL_MAGIC1;          // First byte of magic number
	buffer[PACKET_HEADER_MAGIC2] = PROTOCOL_MAGIC2;          // Second byte of magic number
	buffer[PACKET_HEADER_SOURCE_ID] = data_type;             // Message source identifier
	buffer[PACKET_HEADER_LENGTH_MSB] = (data_length >> 8) & 0xFF; // High byte of data length
	buffer[PACKET_HEADER_LENGTH_LSB] = data_length & 0xFF;        // Low byte of data length
	if (padding > 0) {
		padding = 4 - padding;                         // Calculate the number of padding bytes needed
		memset(buffer + packet_length, 0x00, padding); // Fill the padding bytes with 0x00
	}
	return packet_length + padding; // Return the total size of the packet including header
}

/**
 * @brief Split the passed protocol packet into its component parts
 * 
 * @param message       Pointer to the protocol packet
 * @param data_length   Pointer to a register to receive the packet data size
 * @param packet_type   Pointer to a register to receive the packet type
 * @param data          Pointer to a register to receive the pointer to the packet data content
 */
size_t protocol_split(uint8_t *message, size_t *data_length, protocol_packet_type_e *packet_type, uint8_t **data)
{
	*data_length = (message[PACKET_HEADER_LENGTH_MSB] << 8) | message[PACKET_HEADER_LENGTH_LSB]; // Get the packet size
	*packet_type = (protocol_packet_type_e)message[PACKET_HEADER_SOURCE_ID];                     // Get the packet type
	*data = message + PACKET_HEADER_DATA_START;                                                  // Get the data pointer
	return *data_length + PACKET_HEADER_DATA_START; // Return the packet size
}