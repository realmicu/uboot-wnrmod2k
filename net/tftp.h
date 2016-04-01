/*
 *	LiMon - BOOTP/TFTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 */

#ifndef __TFTP_H__
#define __TFTP_H__

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* tftp.c */
extern void	TftpStart (void);	/* Begin TFTP get */
#ifdef FIRMWARE_RECOVER_FROM_TFTP_SERVER
extern void     TftpServerStart (void);
#endif

/**********************************************************************/

#endif /* __TFTP_H__ */
