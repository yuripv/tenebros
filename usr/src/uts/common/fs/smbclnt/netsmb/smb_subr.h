/*
 * Copyright (c) 2000-2001, Boris Popov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Boris Popov.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: smb_subr.h,v 1.13 2004/09/14 22:59:08 lindak Exp $
 */

/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2018 Nexenta Systems, Inc.  All rights reserved.
 * Copyright 2024 RackTop Systems, Inc.
 */

#ifndef _NETSMB_SMB_SUBR_H_
#define	_NETSMB_SMB_SUBR_H_

#include <sys/cmn_err.h>
#include <sys/lock.h>
#include <sys/note.h>
#include <netsmb/mchain.h>
#include <netsmb/smb_conn.h>

/*
 * Possible lock commands
 */
#define	SMB_LOCK_EXCL		0
#define	SMB_LOCK_SHARED		1
#define	SMB_LOCK_RELEASE	2

struct msgb;	/* avoiding sys/stream.h here */

/* Helper function for SMBERROR */
/*PRINTFLIKE3*/
extern void smb_errmsg(int, const char *, const char *, ...)
	__KPRINTFLIKE(3);
void m_dumpm(struct msgb *);

/*
 * Let's use C99 standard variadic macros!
 * Also the C99 __func__ (function name) feature.
 */
#define	SMBERROR(...) \
	smb_errmsg(CE_NOTE, __func__, __VA_ARGS__)
#define	SMBPANIC(...) \
	smb_errmsg(CE_PANIC, __func__, __VA_ARGS__)
#define	SMBSDEBUG(...) \
	smb_errmsg(CE_CONT, __func__, __VA_ARGS__)
#define	SMBIODEBUG(...) \
	smb_errmsg(CE_CONT, __func__, __VA_ARGS__)
#define	NBDEBUG(...) \
	smb_errmsg(CE_CONT, __func__, __VA_ARGS__)

#if defined(DEBUG) || defined(lint)

#define	DEBUG_ENTER(str) debug_enter(str)

#else /* DEBUG or lint */

#define	DEBUG_ENTER(str) ((void)0)

#endif /* DEBUG or lint */

typedef uint16_t	smb_unichar;
typedef	smb_unichar	*smb_uniptr;

extern smb_unichar smb_unieol;

struct mbchain;
struct smb_rq;
struct smb_vc;

/*
 * These are the attributes we can get from the server via
 * SMB commands such as TRANS2_QUERY_FILE_INFORMATION
 * with info level SMB_QFILEINFO_ALL_INFO, and directory
 * FindFirst/FindNext info. levels FIND_DIRECTORY_INFO
 * and FIND_BOTH_DIRECTORY_INFO, etc.
 *
 * Values in this struct are always native endian,
 * and times are converted converted to Unix form.
 * Note: zero in any of the times means "unknown".
 */
typedef struct smbfattr {
	timespec_t	fa_createtime;	/* Note, != ctime */
	timespec_t	fa_atime;	/* these 3 are like unix */
	timespec_t	fa_mtime;
	timespec_t	fa_ctime;
	u_offset_t	fa_size;	/* EOF position */
	u_offset_t	fa_allocsz;	/* Allocated size. */
	uint32_t	fa_attr;	/* Ext. file (DOS) attr */
} smbfattr_t;

/*
 * Tunable timeout values.  See: smb_smb.c
 */
extern int smb_timo_notice;
extern int smb_timo_default;
extern int smb_timo_open;
extern int smb_timo_read;
extern int smb_timo_write;
extern int smb_timo_append;
extern dev_t nsmb_dev_tcp;
extern dev_t nsmb_dev_tcp6;

/*
 * Tunable timeout values.  See: smb2_smb.c
 */
extern int smb2_timo_notice;
extern int smb2_timo_default;
extern int smb2_timo_open;
extern int smb2_timo_read;
extern int smb2_timo_write;
extern int smb2_timo_append;

void smb_credinit(struct smb_cred *scred, cred_t *cr);
void smb_credrele(struct smb_cred *scred);

int  smb_maperror(int eclass, int eno);
int  smb_maperr32(uint32_t eno);
uint_t smb_doserr2status(int, int);
int smb_get_dstring(struct mdchain *mdc, struct smb_vc *vcp,
	char *outbuf, size_t *outlen, int inlen);
int  smb_put_dmem(struct mbchain *mbp, struct smb_vc *vcp,
    const char *src, int len, int caseopt, int *lenp);
int  smb_put_dstring(struct mbchain *mbp, struct smb_vc *vcp,
    const char *src, int caseopt);
int  smb_put_string(struct smb_rq *rqp, const char *src);
int  smb_put_asunistring(struct smb_rq *rqp, const char *src);

int smb_smb_ntcreate(struct smb_share *ssp, struct mbchain *name_mb,
	uint32_t crflag, uint32_t req_acc, uint32_t efa, uint32_t sh_acc,
	uint32_t disp, uint32_t createopt,  uint32_t impersonate,
	struct smb_cred *scrp, smb_fh_t *fhp,
	uint32_t *cr_act_p, struct smbfattr *fap);

int smb_smb_close(struct smb_share *ssp, smb_fh_t *fhp,
	struct smb_cred *scrp);

int smb_rwuio(smb_fh_t *fhp, uio_rw_t rw,
	uio_t *uiop, smb_cred_t *scred, int timo);

int smb_cmp_sockaddr(struct sockaddr *, struct sockaddr *);
struct sockaddr *smb_dup_sockaddr(struct sockaddr *sa);
void smb_free_sockaddr(struct sockaddr *sa);

int smb_sign_init(struct smb_vc *);
void smb_rq_sign(struct smb_rq *);
int smb_rq_verify(struct smb_rq *);
int smb_calcv2mackey(struct smb_vc *, const uchar_t *,
	const uchar_t *, size_t);
int smb_calcmackey(struct smb_vc *, const uchar_t *,
	const uchar_t *, size_t);

int smb2_sign_init(struct smb_vc *);
void smb2_rq_sign(struct smb_rq *);
int smb2_rq_verify(struct smb_rq *);

void nsmb_crypt_init_mech(struct smb_vc *);
void nsmb_crypt_free_mech(struct smb_vc *);
void nsmb_crypt_init_keys(struct smb_vc *);

int smb3_msg_encrypt(struct smb_vc *vcp, mblk_t **mpp);
int smb3_msg_decrypt(struct smb_vc *vcp, mblk_t **mpp);

/*
 * SMB protocol level functions
 */
int  smb_smb_negotiate(struct smb_vc *vcp, struct smb_cred *scred);
int  smb_smb_ssnsetup(struct smb_vc *vcp, struct smb_cred *scred);
int  smb_smb_logoff(struct smb_vc *vcp, struct smb_cred *scred);
int  smb_smb_echo(smb_vc_t *vcp, smb_cred_t *scred, int timo);
int  smb_smb_treeconnect(smb_share_t *ssp, smb_cred_t *scred);
int  smb_smb_treedisconnect(smb_share_t *ssp, smb_cred_t *scred);

int smb1_smb_ntcreate(struct smb_share *ssp, struct mbchain *name_mb,
	uint32_t crflag, uint32_t req_acc, uint32_t efa, uint32_t sh_acc,
	uint32_t disp, uint32_t createopt,  uint32_t impersonate,
	struct smb_cred *scrp, uint16_t *fidp,
	uint32_t *cr_act_p, struct smbfattr *fap);

int smb1_smb_close(struct smb_share *ssp, uint16_t fid,
	struct timespec *mtime, struct smb_cred *scrp);

int smb_smb_open_prjob(struct smb_share *ssp, char *title,
	uint16_t setuplen, uint16_t mode,
	struct smb_cred *scrp, uint16_t *fidp);

int  smb_smb_close_prjob(struct smb_share *ssp, uint16_t fid,
	struct smb_cred *scrp);

int smb_smb_readx(smb_fh_t *fhp, uint32_t *lenp,
	uio_t *uiop, smb_cred_t *scred, int timo);
int smb_smb_writex(smb_fh_t *fhp, uint32_t *lenp,
	uio_t *uiop, smb_cred_t *scred, int timo);

/*
 * SMB2 protocol level functions
 */
int  smb2_smb_negotiate(struct smb_vc *vcp, struct smb_cred *scred);
int  smb2_smb_ssnsetup(struct smb_vc *vcp, struct smb_cred *scred);
int  smb2_smb_logoff(struct smb_vc *vcp, struct smb_cred *scred);
int  smb2_smb_echo(smb_vc_t *vcp, smb_cred_t *scred, int timo);
int  smb2_smb_treeconnect(smb_share_t *ssp, smb_cred_t *scred);
int  smb2_smb_treedisconnect(smb_share_t *ssp, smb_cred_t *scred);

int
smb2_smb_ntcreate(struct smb_share *ssp, struct mbchain *name_mb,
	struct mbchain *cctx_in, struct mdchain *cctx_out,
	uint32_t crflag, uint32_t req_acc, uint32_t efa, uint32_t sh_acc,
	uint32_t disp, uint32_t createopt,  uint32_t impersonate,
	struct smb_cred *scrp, smb2fid_t *fidp,
	uint32_t *cr_act_p, struct smbfattr *fap);

int  smb2_smb_close(struct smb_share *ssp, smb2fid_t *fid,
	struct smb_cred *scrp);

int smb2_smb_ioctl(struct smb_share *ssp, smb2fid_t *fid,
	struct mbchain *data_in, struct mdchain *data_out,
	uint32_t *data_out_sz,	/* max / returned */
	uint32_t ctl_code, struct smb_cred *scrp);

int smb2_smb_read(smb_fh_t *fhp, uint32_t *lenp,
	uio_t *uiop, smb_cred_t *scred, int timo);
int smb2_smb_write(smb_fh_t *fhp, uint32_t *lenp,
	uio_t *uiop, smb_cred_t *scred, int timo);

/*
 * time conversions
 */

void  smb_time_local2server(struct timespec *tsp, int tzoff, long *seconds);
void  smb_time_server2local(ulong_t seconds, int tzoff, struct timespec *tsp);
void  smb_time_NT2local(uint64_t nsec, struct timespec *tsp);
void  smb_time_local2NT(struct timespec *tsp, uint64_t *nsec);

#endif /* !_NETSMB_SMB_SUBR_H_ */
