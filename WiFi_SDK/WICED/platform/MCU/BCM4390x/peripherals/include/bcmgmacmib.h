/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *
 * Hardware-specific MIB definition for
 * Broadcom Home Networking Division
 * GbE Unimac core
 *
 * $Id: bcmgmacmib.h 376342 2012-12-24 21:02:49Z palter $
 */

#ifndef	_bcmgmacmib_h_
#define	_bcmgmacmib_h_


/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

/* GMAC MIB structure */

typedef struct _gmacmib {
	uint32	tx_good_octets;		/* 0x300 */
	uint32	tx_good_octets_high;	/* 0x304 */
	uint32	tx_good_pkts;		/* 0x308 */
	uint32	tx_octets;		/* 0x30c */
	uint32	tx_octets_high;		/* 0x310 */
	uint32	tx_pkts;		/* 0x314 */
	uint32	tx_broadcast_pkts;	/* 0x318 */
	uint32	tx_multicast_pkts;	/* 0x31c */
	uint32	tx_len_64;		/* 0x320 */
	uint32	tx_len_65_to_127;	/* 0x324 */
	uint32	tx_len_128_to_255;	/* 0x328 */
	uint32	tx_len_256_to_511;	/* 0x32c */
	uint32	tx_len_512_to_1023;	/* 0x330 */
	uint32	tx_len_1024_to_1522;	/* 0x334 */
	uint32	tx_len_1523_to_2047;	/* 0x338 */
	uint32	tx_len_2048_to_4095;	/* 0x33c */
	uint32	tx_len_4095_to_8191;	/* 0x340 */
	uint32	tx_len_8192_to_max;	/* 0x344 */
	uint32	tx_jabber_pkts;		/* 0x348 */
	uint32	tx_oversize_pkts;	/* 0x34c */
	uint32	tx_fragment_pkts;	/* 0x350 */
	uint32	tx_underruns;		/* 0x354 */
	uint32	tx_total_cols;		/* 0x358 */
	uint32	tx_single_cols;		/* 0x35c */
	uint32	tx_multiple_cols;	/* 0x360 */
	uint32	tx_excessive_cols;	/* 0x364 */
	uint32	tx_late_cols;		/* 0x368 */
	uint32	tx_defered;		/* 0x36c */
	uint32	tx_carrier_lost;	/* 0x370 */
	uint32	tx_pause_pkts;		/* 0x374 */
	uint32	tx_uni_pkts;		/* 0x378 */
	uint32	tx_q0_pkts;		/* 0x37c */
	uint32	tx_q0_octets;		/* 0x380 */
	uint32	tx_q0_octets_high;	/* 0x384 */
	uint32	tx_q1_pkts;		/* 0x388 */
	uint32	tx_q1_octets;		/* 0x38c */
	uint32	tx_q1_octets_high;	/* 0x390 */
	uint32	tx_q2_pkts;		/* 0x394 */
	uint32	tx_q2_octets;		/* 0x398 */
	uint32	tx_q2_octets_high;	/* 0x39c */
	uint32	tx_q3_pkts;		/* 0x3a0 */
	uint32	tx_q3_octets;		/* 0x3a4 */
	uint32	tx_q3_octets_high;	/* 0x3a8 */
	uint32	PAD;
	uint32	rx_good_octets;		/* 0x3b0 */
	uint32	rx_good_octets_high;	/* 0x3b4 */
	uint32	rx_good_pkts;		/* 0x3b8 */
	uint32	rx_octets;		/* 0x3bc */
	uint32	rx_octets_high;		/* 0x3c0 */
	uint32	rx_pkts;		/* 0x3c4 */
	uint32	rx_broadcast_pkts;	/* 0x3c8 */
	uint32	rx_multicast_pkts;	/* 0x3cc */
	uint32	rx_len_64;		/* 0x3d0 */
	uint32	rx_len_65_to_127;	/* 0x3d4 */
	uint32	rx_len_128_to_255;	/* 0x3d8 */
	uint32	rx_len_256_to_511;	/* 0x3dc */
	uint32	rx_len_512_to_1023;	/* 0x3e0 */
	uint32	rx_len_1024_to_1522;	/* 0x3e4 */
	uint32	rx_len_1523_to_2047;	/* 0x3e8 */
	uint32	rx_len_2048_to_4095;	/* 0x3ec */
	uint32	rx_len_4095_to_8191;	/* 0x3f0 */
	uint32	rx_len_8192_to_max;	/* 0x3f4 */
	uint32	rx_jabber_pkts;		/* 0x3f8 */
	uint32	rx_oversize_pkts;	/* 0x3fc */
	uint32	rx_fragment_pkts;	/* 0x400 */
	uint32	rx_missed_pkts;		/* 0x404 */
	uint32	rx_crc_align_errs;	/* 0x408 */
	uint32	rx_undersize;		/* 0x40c */
	uint32	rx_crc_errs;		/* 0x410 */
	uint32	rx_align_errs;		/* 0x414 */
	uint32	rx_symbol_errs;		/* 0x418 */
	uint32	rx_pause_pkts;		/* 0x41c */
	uint32	rx_nonpause_pkts;	/* 0x420 */
	uint32	rx_sachanges;		/* 0x424 */
	uint32	rx_uni_pkts;		/* 0x428 */
} gmacmib_t;

#define	GM_MIB_BASE		0x300
#define	GM_MIB_LIMIT		0x800

#endif	/* _bcmgmacmib_h_ */
