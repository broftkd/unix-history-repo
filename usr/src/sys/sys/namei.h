/*	namei.h	6.2	84/01/03	*/

struct namidata {
	int	ni_offset;
	int	ni_count;
	struct	inode *ni_pdir;
	struct	direct ni_dent;
};

enum nami_op { NAMI_LOOKUP, NAMI_CREATE, NAMI_DELETE };

/* this is temporary until the namei interface changes */
#define	LOOKUP		0	/* perform name lookup only */
#define	CREATE		1	/* setup for file creation */
#define	DELETE		2	/* setup for file deletion */
#define	LOCKPARENT	0x10	/* see the top of namei */
#define NOCACHE		0x20	/* name must not be left in cache */

/*
 * This structure describes the elements in the cache of recent
 * names looked up by namei.
 */
#define	NCHNAMLEN	11	/* maximum name segment length we bother with */
struct	nch {
	struct	nch	*nc_forw, *nc_back;	/* hash chain, MUST BE FIRST */
	struct	nch	*nc_nxt, **nc_prev;	/* LRU chain */
	struct	inode	*nc_ip;			/* inode the name refers to */
	ino_t		 nc_ino;		/* ino of parent of name */
	dev_t		 nc_dev;		/* dev of parent of name */
	dev_t		 nc_idev;		/* dev of the name ref'd */
	char		 nc_nlen;		/* length of name */
	char		 nc_name[NCHNAMLEN];	/* segment name */
};
struct	nch *nch;
int	nchsize;
