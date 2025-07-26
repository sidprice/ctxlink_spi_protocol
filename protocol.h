/**
 * @file protocol.h
 * @author Sid Price (sid@sidprice.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-02
 * 
 * @copyright Copyright Sid Price (c) 2025
 * 
 * Definitions and references dealing with the protocol used for
 * communications between the ESP32 and ctxLink
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ESP32_BUILD
#include <Arduino.h>
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

/*
     This file describes the packet protocol used between an ESP32 based
     Wi-FI module and ctxLink using SPI peripherals.
 
     ctxLink is the master, and ESP32 is the slave module.
 
     +---------------------------------+
     |  Magic Number (2 bytes)         |
     +---------------------------------+
     |  Packet Type (1 byte)           |
     +---------------------------------+
     |  Following bytecount (2 bytes)  |
     +---------------------------------+
     |  Data (variable length)         |
     +---------------------------------+
 
     NOTE:   The total packet size MUST be a multiple of 4-bytes as
             required by the ESP32 DMA peripheral. If required, dummy
             bytes of 0x00 should be appended.
 */

/**
  * @brief Protocol Packet Header offsets
  * 
  */
enum packet_header_offset_e {
	PACKET_HEADER_MAGIC1 = 0, // First byte of the magic number
	PACKET_HEADER_MAGIC2,     // Second byte of the magic number
	PACKET_HEADER_SOURCE_ID,  // Message source identifier (e.g. 0x01 for GDB
	PACKET_HEADER_LENGTH_MSB, // Most significant byte of the length of the data
	PACKET_HEADER_LENGTH_LSB, // Least significant byte of the length of the data)
	PACKET_HEADER_DATA_START  // Start of the actual data in the packet
};

/**
  * @brief Definition of the protocol magic number, used in the header
  * 
  */
#define PROTOCOL_MAGIC1 0xDE; // First byte of the magic number
#define PROTOCOL_MAGIC2 0xAD; // Second byte of the magic number

typedef enum {
	PROTOCOL_PACKET_TYPE_EMPTY = 0x00,      // 0x00 Empty packet type
	PROTOCOL_PACKET_TYPE_CMD,               // 0x01 Command from ctxLink
	PROTOCOL_PACKET_TYPE_STATUS,            // 0x02 Status of ESP32 to ctxLink
	PROTOCOL_PACKET_TYPE_FROM_GDB,          // 0x03 Packet from GDB
	PROTOCOL_PACKET_TYPE_TO_GDB,            // 0x04 Packet to GDB
	PROTOCOL_PACKET_TYPE_FROM_CTXLINK,      // 0x05 Packet from ctxLink
	PROTOCOL_PACKET_TYPE_TO_CTXLINK,        // 0x06 Packet to ctxLink
	PROTOCOL_PACKET_TYPE_SWO,               // 0x07 SWO packet type - to network
	PROTOCOL_PACKET_TYPE_UART_FROM_CTXLINK, // 0x08 UART packet from ctxLink
	PROTOCOL_PACKET_TYPE_UART_TO_CTXLINK,   // 0x09 UART packet to ctxLink
	PROTOCOL_PACKET_TYPE_NETWORK_INFO,      // 0x0a Network information packet type
	PROTOCOL_PACKET_TYPE_SET_NETWORK_INFO,  // 0x0b Set network information packet type
} protocol_packet_type_e;

/*
     Define the format of the ESP32 status packet to ctxLink
     
     The status packet is used to inform ctxLink of the status of the ESP32, it has the following format:
 
     +---------------------------------+
     |     Status Type (1 byte)        |     (implicit data byte count)
     +---------------------------------+
     |        Status Value 0           |
     +---------------------------------+
     |               ...               |
     +---------------------------------+
     |        Status Value n           |
     +---------------------------------+
  */

typedef enum {
	PROTOCOL_PACKET_STATUS_TYPE_WIFI = 0x00,    // 0x00 State of Wi-Fi connection, 0x01 = connected, 0x00 = disconnected
	PROTOCOL_PACKET_STATUS_TYPE_GDB_CLIENT,     // 0x01 GDB client connected, 0x01 = connected, 0x00 = disconnected
	PROTOCOL_PACKET_STATUS_TYPE_UART_CLIENT,    // 0x02 UART client connected, 0x01 = connected, 0x00 = disconnected
	PROTOCOL_PACKET_STATUS_TYPE_SWO_CLIENT,     // 0x03 SWO client connected, 0x01 = connected, 0x00 = disconnected
	PROTOCOL_PACKET_STATUS_TYPE_NETWORK_CLIENT, // 0x04 Network data structure describes the current network
} protocol_packet_status_type_e;

#define MAX_SSID_LENGTH 64 // Maximum length of the SSID string
#define MAX_PASS_PHRASE_LENGTH 64 // Maximum length of the passphrase string

/**
  * @brief Structure used to hold network connection data
  * 
  */
typedef struct {
	uint8_t type;                        // Type of status packet
	uint8_t connected;                   // 0x00 = disconnected, 0x01 = connected
	char network_ssid[MAX_SSID_LENGTH]; // AP connection SSID name.  Only valid in
										 //  station mode.  Will be NULL in SoftAP
										 //  mode or P2P mode.
	uint8_t ip_address[4];               // connection IP address
	uint8_t mac_address[6];              // MAC address of the peer Wi-Fi station
	int8_t rssi;
} network_connection_info_s;

typedef struct {
	uint8_t type;   // protocol_packet_status_type_e Type of status packet
	uint8_t status; // 0x00 = disconnected, 0x01 = connected
} protocol_packet_status_s;

size_t package_data(uint8_t *buffer, size_t data_length, protocol_packet_type_e data_type);
size_t protocol_split(uint8_t *message, size_t *packet_size, protocol_packet_type_e *packet_type, uint8_t **data);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H