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
 * Fundamental constants relating to ICMP Protocol
 *
 * $Id: bcmicmp.h 384540 2013-02-12 04:28:58Z automrgr $
 */

#ifndef _bcmicmp_h_
#define _bcmicmp_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>


#define ICMP_TYPE_ECHO_REQUEST	8	/* ICMP type echo request */
#define ICMP_TYPE_ECHO_REPLY		0	/* ICMP type echo reply */

#define ICMP_CHKSUM_OFFSET	2	/* ICMP body checksum offset */

/* ICMP6 error and control message types */
#define ICMP6_DEST_UNREACHABLE		1
#define ICMP6_PKT_TOO_BIG		2
#define ICMP6_TIME_EXCEEDED		3
#define ICMP6_PARAM_PROBLEM		4
#define ICMP6_ECHO_REQUEST		128
#define ICMP6_ECHO_REPLY		129
#define ICMP_MCAST_LISTENER_QUERY	130
#define ICMP_MCAST_LISTENER_REPORT	131
#define ICMP_MCAST_LISTENER_DONE	132
#define ICMP6_RTR_SOLICITATION		133
#define ICMP6_RTR_ADVERTISEMENT		134
#define ICMP6_NEIGH_SOLICITATION	135
#define ICMP6_NEIGH_ADVERTISEMENT	136
#define ICMP6_REDIRECT			137

#define ICMP6_RTRSOL_OPT_OFFSET		8
#define ICMP6_RTRADV_OPT_OFFSET		16
#define ICMP6_NEIGHSOL_OPT_OFFSET	24
#define ICMP6_NEIGHADV_OPT_OFFSET	24
#define ICMP6_REDIRECT_OPT_OFFSET	40

BWL_PRE_PACKED_STRUCT struct icmp6_opt {
	uint8	type;		/* Option identifier */
	uint8	length;		/* Lenth including type and length */
	uint8	data[0];	/* Variable length data */
} BWL_POST_PACKED_STRUCT;

#define	ICMP6_OPT_TYPE_SRC_LINK_LAYER	1
#define	ICMP6_OPT_TYPE_TGT_LINK_LAYER	2
#define	ICMP6_OPT_TYPE_PREFIX_INFO	3
#define	ICMP6_OPT_TYPE_REDIR_HDR	4
#define	ICMP6_OPT_TYPE_MTU		5

/* These fields are stored in network order */
BWL_PRE_PACKED_STRUCT struct bcmicmp_hdr {
	uint8	type;		/* Echo or Echo-reply */
	uint8	code;		/* Always 0 */
	uint16	chksum;		/* Icmp packet checksum */
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* #ifndef _bcmicmp_h_ */
