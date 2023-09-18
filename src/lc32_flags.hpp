// LC32 modes for open()
#define		LC32_S_IRUSR	0000400	/* read permission, owner */
#define		LC32_S_IWUSR	0000200	/* write permission, owner */
#define		LC32_S_IXUSR    0000100/* execute/search permission, owner */
#define		LC32_S_IRGRP	0000040	/* read permission, group */
#define		LC32_S_IWGRP	0000020	/* write permission, grougroup */
#define		LC32_S_IXGRP    0000010/* execute/search permission, group */
#define		LC32_S_IROTH	0000004	/* read permission, other */
#define		LC32_S_IWOTH	0000002	/* write permission, other */
#define		LC32_S_IXOTH    0000001/* execute/search permission, other */

// LC32 flags for open()
#define	    LC32_O_RDONLY	0		/* +1 == FREAD */
#define	    LC32_O_WRONLY	1		/* +1 == FWRITE */
#define	    LC32_O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	    LC32_O_APPEND	0x0008
#define	    LC32_O_CREAT	0x0200
#define	    LC32_O_TRUNC	0x0400

// LC32 file descriptors
#define LC32_STDIN_FILENO    0       /* standard input file descriptor */
#define LC32_STDOUT_FILENO   1       /* standard output file descriptor */
#define LC32_STDERR_FILENO   2       /* standard error file descriptor */

// LC32 errno
