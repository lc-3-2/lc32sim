#pragma once

// LC32 file modes
#define	    LC32_S_ISUID	0004000	/* set user id on execution */
#define	    LC32_S_ISGID	0002000	/* set group id on execution */
#define	    LC32_S_ISVTX	0001000	/* save swapped text even after use */
#define		LC32_S_IRUSR	0000400	/* read permission, owner */
#define		LC32_S_IWUSR	0000200	/* write permission, owner */
#define		LC32_S_IXUSR    0000100/* execute/search permission, owner */
#define		LC32_S_IRGRP	0000040	/* read permission, group */
#define		LC32_S_IWGRP	0000020	/* write permission, grougroup */
#define		LC32_S_IXGRP    0000010/* execute/search permission, group */
#define		LC32_S_IROTH	0000004	/* read permission, other */
#define		LC32_S_IWOTH	0000002	/* write permission, other */
#define		LC32_S_IXOTH    0000001/* execute/search permission, other */

// LC32 file flags
#define	    LC32_O_RDONLY	0		/* +1 == FREAD */
#define	    LC32_O_WRONLY	1		/* +1 == FWRITE */
#define	    LC32_O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	    LC32_O_APPEND	0x0008
#define	    LC32_O_CREAT	0x0200
#define	    LC32_O_TRUNC	0x0400

// LC32 file descriptors
#define     LC32_STDIN_FILENO    0       /* standard input file descriptor */
#define     LC32_STDOUT_FILENO   1       /* standard output file descriptor */
#define     LC32_STDERR_FILENO   2       /* standard error file descriptor */

// LC32 file types
#define	    LC32_S_IFMT	    0170000	/* type of file */
#define		LC32_S_IFDIR	0040000	/* directory */
#define		LC32_S_IFCHR	0020000	/* character special */
#define		LC32_S_IFBLK	0060000	/* block special */
#define		LC32_S_IFREG	0100000	/* regular */
#define		LC32_S_IFLNK	0120000	/* symbolic link */
#define		LC32_S_IFSOCK	0140000	/* socket */
#define		LC32_S_IFIFO	0010000	/* fifo */

// LC32 seek types
# define	LC32_SEEK_SET	0
# define	LC32_SEEK_CUR	1
# define	LC32_SEEK_END	2

// LC32 errno (all system dependent errno excluded)
#define	    LC32_EPERM 1		/* Not owner */
#define	    LC32_ENOENT 2	/* No such file or directory */
#define	    LC32_ESRCH 3		/* No such process */
#define	    LC32_EINTR 4		/* Interrupted system call */
#define	    LC32_EIO 5		/* I/O error */
#define	    LC32_ENXIO 6		/* No such device or address */
#define	    LC32_E2BIG 7		/* Arg list too long */
#define	    LC32_ENOEXEC 8	/* Exec format error */
#define	    LC32_EBADF 9		/* Bad file number */
#define	    LC32_ECHILD 10	/* No children */
#define	    LC32_EAGAIN 11	/* No more processes */
#define	    LC32_ENOMEM 12	/* Not enough space */
#define	    LC32_EACCES 13	/* Permission denied */
#define	    LC32_EFAULT 14	/* Bad address */
#define	    LC32_EBUSY 16	/* Device or resource busy */
#define	    LC32_EEXIST 17	/* File exists */
#define	    LC32_EXDEV 18	/* Cross-device link */
#define	    LC32_ENODEV 19	/* No such device */
#define	    LC32_ENOTDIR 20	/* Not a directory */
#define	    LC32_EISDIR 21	/* Is a directory */
#define	    LC32_EINVAL 22	/* Invalid argument */
#define	    LC32_ENFILE 23	/* Too many open files in system */
#define	    LC32_EMFILE 24	/* File descriptor value too large */
#define	    LC32_ENOTTY 25	/* Not a character device */
#define	    LC32_ETXTBSY 26	/* Text file busy */
#define	    LC32_EFBIG 27	/* File too large */
#define	    LC32_ENOSPC 28	/* No space left on device */
#define	    LC32_ESPIPE 29	/* Illegal seek */
#define	    LC32_EROFS 30	/* Read-only file system */
#define	    LC32_EMLINK 31	/* Too many links */
#define	    LC32_EPIPE 32	/* Broken pipe */
#define	    LC32_EDOM 33		/* Mathematics argument out of domain of function */
#define	    LC32_ERANGE 34	/* Result too large */
#define	    LC32_ENOMSG 35	/* No message of desired type */
#define	    LC32_EIDRM 36	/* Identifier removed */
#define	    LC32_EDEADLK 45	/* Deadlock */
#define	    LC32_ENOLCK 46	/* No lock */
#define     LC32_ENOSTR 60	/* Not a stream */
#define     LC32_ENODATA 61	/* No data (for no delay io) */
#define     LC32_ETIME 62	/* Stream ioctl timeout */
#define     LC32_ENOSR 63	/* No stream resources */
#define     LC32_ENOLINK 67	/* Virtual circuit is gone */
#define     LC32_EPROTO 71	/* Protocol error */
#define	    LC32_EMULTIHOP 74	/* Multihop attempted */
#define     LC32_EBADMSG 77	/* Bad message */
#define     LC32_EFTYPE 79	/* Inappropriate file type or format */
#define     LC32_ENOSYS 88	/* Function not implemented */
#define     LC32_ENOTEMPTY 90	/* Directory not empty */
#define     LC32_ENAMETOOLONG 91	/* File or path name too long */
#define     LC32_ELOOP 92	/* Too many symbolic links */
#define     LC32_EOPNOTSUPP 95	/* Operation not supported on socket */
#define     LC32_EPFNOSUPPORT 96 /* Protocol family not supported */
#define     LC32_ECONNRESET 104  /* Connection reset by peer */
#define     LC32_ENOBUFS 105	/* No buffer space available */
#define     LC32_EAFNOSUPPORT 106 /* Address family not supported by protocol family */
#define     LC32_EPROTOTYPE 107	/* Protocol wrong type for socket */
#define     LC32_ENOTSOCK 108	/* Socket operation on non-socket */
#define     LC32_ENOPROTOOPT 109	/* Protocol not available */
#define     LC32_ECONNREFUSED 111	/* Connection refused */
#define     LC32_EADDRINUSE 112		/* Address already in use */
#define     LC32_ECONNABORTED 113	/* Software caused connection abort */
#define     LC32_ENETUNREACH 114		/* Network is unreachable */
#define     LC32_ENETDOWN 115		/* Network interface is not configured */
#define     LC32_ETIMEDOUT 116		/* Connection timed out */
#define     LC32_EHOSTDOWN 117		/* Host is down */
#define     LC32_EHOSTUNREACH 118	/* Host is unreachable */
#define     LC32_EINPROGRESS 119		/* Connection already in progress */
#define     LC32_EALREADY 120		/* Socket already connected */
#define     LC32_EDESTADDRREQ 121	/* Destination address required */
#define     LC32_EMSGSIZE 122		/* Message too long */
#define     LC32_EPROTONOSUPPORT 123	/* Unknown protocol */
#define     LC32_EADDRNOTAVAIL 125	/* Address not available */
#define     LC32_ENETRESET 126		/* Connection aborted by network */
#define     LC32_EISCONN 127		/* Socket is already connected */
#define     LC32_ENOTCONN 128		/* Socket is not connected */
#define     LC32_ETOOMANYREFS 129
#define     LC32_EDQUOT 132
#define     LC32_ESTALE 133
#define     LC32_ENOTSUP 134		/* Not supported */
#define     LC32_EILSEQ 138		/* Illegal byte sequence */
#define     LC32_EOVERFLOW 139	/* Value too large for defined data type */
#define     LC32_ECANCELED 140	/* Operation canceled */
#define     LC32_ENOTRECOVERABLE 141	/* State not recoverable */
#define     LC32_EOWNERDEAD 142	/* Previous owner died */