/*
 * The new sysinstall program.
 *
 * This is probably the last attempt in the `sysinstall' line, the next
 * generation being slated to essentially a complete rewrite.
 *
 * $Id: sysinstall.h,v 1.59 1996/06/11 09:47:30 jkh Exp $
 *
 * Copyright (c) 1995
 *	Jordan Hubbard.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    verbatim and that no modifications are made prior to this
 *    point in the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JORDAN HUBBARD ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JORDAN HUBBARD OR HIS PETS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, LIFE OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _SYSINSTALL_H_INCLUDE
#define _SYSINSTALL_H_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dialog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "libdisk.h"
#include "dist.h"
#include "version.h"

/*** Defines ***/

/* variable limits */
#define VAR_NAME_MAX		128
#define VAR_VALUE_MAX		1024

/* device limits */
#define DEV_NAME_MAX		64	/* The maximum length of a device name	*/
#define DEV_MAX			100	/* The maximum number of devices we'll deal with */
#define INTERFACE_MAX		50	/* Maximum number of network interfaces we'll deal with */
#define MAX_FTP_RETRIES		"4"	/* How many times to beat our heads against the wall */

/*
 * I make some pretty gross assumptions about having a max of 50 chunks
 * total - 8 slices and 42 partitions.  I can't easily display many more
 * than that on the screen at once!
 *
 * For 2.1 I'll revisit this and try to make it more dynamic, but since
 * this will catch 99.99% of all possible cases, I'm not too worried.
 */
#define MAX_CHUNKS	40

/* Internal environment variable names */
#define DISK_PARTITIONED		"_diskPartitioned"
#define DISK_LABELLED			"_diskLabelled"
#define DISK_SELECTED			"_diskSelected"
#define SYSTEM_STATE			"_systemState"
#define RUNNING_ON_ROOT			"_runningOnRoot"
#define TCP_CONFIGURED			"_tcpConfigured"

/* Ones that can be tweaked from config files */
#define VAR_BLANKTIME			"blanktime"
#define VAR_BOOTMGR			"bootManager"
#define VAR_BROWSER_BINARY		"browserBinary"
#define VAR_BROWSER_PACKAGE		"browserPackage"
#define VAR_CONFIG_FILE			"configFile"
#define VAR_CPIO_VERBOSITY		"cpioVerbose"
#define VAR_DEBUG			"debug"
#define VAR_DISK			"disk"
#define VAR_DISKSPACE			"diskSpace"
#define VAR_DOMAINNAME			"domainname"
#define VAR_EDITOR			"editor"
#define VAR_EXTRAS			"ifconfig_"
#define VAR_FTP_ONERROR			"ftpOnError"
#define VAR_FTP_PASS			"ftpPass"
#define VAR_FTP_PATH			"ftp"
#define VAR_FTP_RETRIES			"ftpRetryCount"
#define VAR_FTP_STATE			"ftpState"
#define VAR_FTP_USER			"ftpUser"
#define VAR_GATEWAY			"defaultrouter"
#define VAR_GEOMETRY			"geometry"
#define VAR_HOSTNAME			"hostname"
#define VAR_IFCONFIG			"ifconfig_"
#define VAR_INTERFACES			"network_interfaces"
#define VAR_IPADDR			"ipaddr"
#define VAR_LABEL			"label"
#define VAR_LABEL_COUNT			"labelCount"
#define VAR_MEDIA_TYPE			"mediaType"
#define VAR_NAMESERVER			"nameserver"
#define VAR_NETMASK			"netmask"
#define VAR_NFS_PATH			"nfs"
#define VAR_NFS_SECURE			"nfsSecure"
#define VAR_NFS_SERVER			"nfs_server"
#define VAR_NO_CONFIRM			"noConfirm"
#define VAR_NTPDATE			"ntpDate"
#define VAR_PORTS_PATH			"ports"
#define VAR_RELNAME			"releaseName"
#define VAR_ROOT_SIZE			"rootSize"
#define VAR_ROUTEDFLAGS			"routedflags"
#define VAR_GATED			"gated"
#define VAR_PCNFSD			"pcnfsd"
#define VAR_SLOW_ETHER			"slowEthernetCard"
#define VAR_SWAP_SIZE			"swapSize"
#define VAR_TAPE_BLOCKSIZE		"tapeBlocksize"
#define VAR_UFS_PATH			"ufs"
#define VAR_USR_SIZE			"usrSize"
#define VAR_VAR_SIZE			"varSize"
#define VAR_SERIAL_SPEED		"serialSpeed"

#define DEFAULT_TAPE_BLOCKSIZE	"20"

/* One MB worth of blocks */
#define ONE_MEG				2048

/* Which selection attributes to use */
#define ATTR_SELECTED			(ColorDisplay ? item_selected_attr : item_attr)

/*** Types ***/
typedef unsigned int Boolean;
typedef struct disk Disk;
typedef struct chunk Chunk;

/* Bitfields for menu options */
#define DMENU_NORMAL_TYPE	0x1     /* Normal dialog menu           */
#define DMENU_RADIO_TYPE	0x2     /* Radio dialog menu            */
#define DMENU_CHECKLIST_TYPE	0x4     /* Multiple choice menu         */
#define DMENU_SELECTION_RETURNS 0x8     /* Immediate return on item selection */

typedef struct _dmenu {
    int type;				/* What sort of menu we are	*/
    char *title;			/* Our title			*/
    char *prompt;			/* Our prompt			*/
    char *helpline;			/* Line of help at bottom	*/
    char *helpfile;			/* Help file for "F1"		*/
    dialogMenuItem items[0];		/* Array of menu items		*/
} DMenu;

/* A sysconfig variable */
typedef struct _variable {
    struct _variable *next;
    char name[VAR_NAME_MAX];
    char value[VAR_VALUE_MAX];
} Variable;

/* For attribs */
#define MAX_ATTRIBS	200
#define MAX_NAME	64
#define MAX_VALUE	256

typedef struct _attribs {
    char name[MAX_NAME];
    char value[MAX_VALUE];
} Attribs;

typedef enum {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_DISK,
    DEVICE_TYPE_FLOPPY,
    DEVICE_TYPE_FTP,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_CDROM,
    DEVICE_TYPE_TAPE,
    DEVICE_TYPE_DOS,
    DEVICE_TYPE_UFS,
    DEVICE_TYPE_NFS,
    DEVICE_TYPE_ANY,
} DeviceType;

/* A "device" from sysinstall's point of view */
typedef struct _device {
    char name[DEV_NAME_MAX];
    char *description;
    char *devname;
    DeviceType type;
    Boolean enabled;
    Boolean (*init)(struct _device *dev);
    int (*get)(struct _device *dev, char *file, Boolean probe);
    Boolean (*close)(struct _device *dev, int fd);
    void (*shutdown)(struct _device *dev);
    void *private;
    unsigned int flags;
} Device;

/* Some internal representations of partitions */
typedef enum {
    PART_NONE,
    PART_SLICE,
    PART_SWAP,
    PART_FILESYSTEM,
    PART_FAT,
} PartType;

/* The longest newfs command we'll hand to system() */
#define NEWFS_CMD_MAX	256

typedef struct _part_info {
    Boolean newfs;
    char mountpoint[FILENAME_MAX];
    char newfs_cmd[NEWFS_CMD_MAX];
} PartInfo;

/* An option */
typedef struct _opt {
    char *name;
    char *desc;
    enum { OPT_IS_STRING, OPT_IS_INT, OPT_IS_FUNC, OPT_IS_VAR } type;
    void *data;
    void *aux;
    char *(*check)();
} Option;

/* Weird index nodey things we use for keeping track of package information */
typedef enum { PACKAGE, PLACE } node_type;	/* Types of nodes */

typedef struct _pkgnode {	/* A node in the reconstructed hierarchy */
    struct _pkgnode *next;	/* My next sibling			*/
    node_type type;		/* What am I?				*/
    char *name;			/* My name				*/
    char *desc;			/* My description (Hook)		*/
    struct _pkgnode *kids;	/* My little children			*/
    void *data;			/* A place to hang my data		*/
} PkgNode;
typedef PkgNode *PkgNodePtr;

/* A single package */
typedef struct _indexEntry {	/* A single entry in an INDEX file */
    char *name;			/* name				*/
    char *path;			/* full path to port		*/
    char *prefix;		/* port prefix			*/
    char *comment;		/* one line description		*/
    char *descrfile;		/* path to description file	*/
    char *deps;			/* packages this depends on	*/
    char *maintainer;		/* maintainer			*/
} IndexEntry;
typedef IndexEntry *IndexEntryPtr;

typedef int (*commandFunc)(char *key, void *data);

#define HOSTNAME_FIELD_LEN	128
#define IPADDR_FIELD_LEN	16
#define EXTRAS_FIELD_LEN	128

/* This is the structure that Network devices carry around in their private, erm, structures */
typedef struct _devPriv {
    char ipaddr[IPADDR_FIELD_LEN];
    char netmask[IPADDR_FIELD_LEN];
    char extras[EXTRAS_FIELD_LEN];
} DevInfo;


/*** Externs ***/
extern int		DebugFD;		/* Where diagnostic output goes			*/
extern Boolean		Fake;			/* Don't actually modify anything - testing	*/
extern Boolean		SystemWasInstalled;	/* Did we install it?				*/
extern Boolean		RunningAsInit;		/* Are we running stand-alone?			*/
extern Boolean		DialogActive;		/* Is the dialog() stuff up?			*/
extern Boolean		ColorDisplay;		/* Are we on a color display?			*/
extern Boolean		OnVTY;			/* On a syscons VTY?				*/
extern Variable		*VarHead;		/* The head of the variable chain		*/
extern Device		*mediaDevice;		/* Where we're getting our distribution from	*/
extern unsigned int	Dists;			/* Which distributions we want			*/
extern unsigned int	DESDists;		/* Which naughty distributions we want		*/
extern unsigned int	SrcDists;		/* Which src distributions we want		*/
extern unsigned int	XF86Dists;		/* Which XFree86 dists we want			*/
extern unsigned int	XF86ServerDists;	/* The XFree86 servers we want			*/
extern unsigned int	XF86FontDists;		/* The XFree86 fonts we want			*/
extern int		BootMgr;		/* Which boot manager to use 			*/
extern DMenu		MenuInitial;		/* Initial installation menu			*/
extern DMenu		MenuFixit;		/* Fixit repair menu				*/
extern DMenu		MenuMBRType;		/* Type of MBR to write on the disk		*/
extern DMenu		MenuConfigure;		/* Final configuration menu			*/
extern DMenu		MenuDocumentation;	/* Documentation menu				*/
extern DMenu		MenuFTPOptions;		/* FTP Installation options			*/
extern DMenu		MenuOptions;		/* Installation options				*/
extern DMenu		MenuOptionsLanguage;	/* Language options menu			*/
extern DMenu		MenuMedia;		/* Media type menu				*/
extern DMenu		MenuMouse;		/* Mouse type menu				*/
extern DMenu		MenuMediaCDROM;		/* CDROM media menu				*/
extern DMenu		MenuMediaDOS;		/* DOS media menu				*/
extern DMenu		MenuMediaFloppy;	/* Floppy media menu				*/
extern DMenu		MenuMediaFTP;		/* FTP media menu				*/
extern DMenu		MenuMediaTape;		/* Tape media menu				*/
extern DMenu		MenuNetworkDevice;	/* Network device menu				*/
extern DMenu		MenuNTP;		/* NTP time server menu				*/
extern DMenu		MenuSyscons;		/* System console configuration menu		*/
extern DMenu		MenuSysconsKeymap;	/* System console keymap configuration menu	*/
extern DMenu		MenuSysconsKeyrate;	/* System console keyrate configuration menu	*/
extern DMenu		MenuSysconsSaver;	/* System console saver configuration menu	*/
extern DMenu		MenuNetworking;		/* Network configuration menu			*/
extern DMenu		MenuInstallCustom;	/* Custom Installation menu			*/
extern DMenu		MenuDistributions;	/* Distribution menu				*/
extern DMenu		MenuSubDistributions;	/* Custom distribution menu			*/
extern DMenu		MenuDESDistributions;	/* DES distribution menu			*/
extern DMenu		MenuSrcDistributions;	/* Source distribution menu			*/
extern DMenu		MenuXF86;		/* XFree86 main menu				*/
extern DMenu		MenuXF86Select;		/* XFree86 distribution selection menu		*/
extern DMenu		MenuXF86SelectCore;	/* XFree86 core distribution menu		*/
extern DMenu		MenuXF86SelectServer;	/* XFree86 server distribution menu		*/
extern DMenu		MenuXF86SelectFonts;	/* XFree86 font selection menu			*/
extern DMenu		MenuDiskDevices;	/* Disk devices menu				*/
extern DMenu		MenuHTMLDoc;		/* HTML Documentation menu			*/


/*** Prototypes ***/

/* apache.c */
extern int	configApache(dialogMenuItem *self);

/* anonFTP.c */
extern int	configAnonFTP(dialogMenuItem *self);

/* attrs.c */
extern char	*attr_match(Attribs *attr, char *name);
extern int	attr_parse_file(Attribs *attr, char *file);
extern int	attr_parse(Attribs *attr, int fd);

/* cdrom.c */
extern Boolean	mediaInitCDROM(Device *dev);
extern int	mediaGetCDROM(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownCDROM(Device *dev);

/* command.c */
extern void	command_clear(void);
extern void	command_sort(void);
extern void	command_execute(void);
extern void	command_shell_add(char *key, char *fmt, ...);
extern void	command_func_add(char *key, commandFunc func, void *data);

/* config.c */
extern int	configFstab(void);
extern void	configSysconfig(void);
extern void	configResolv(void);
extern int	configPorts(dialogMenuItem *self);
extern int	configPackages(dialogMenuItem *self);
extern int	configSaverTimeout(dialogMenuItem *self);
extern int	configNTP(dialogMenuItem *self);
extern int	configXFree86(dialogMenuItem *self);
extern int	configRoutedFlags(dialogMenuItem *self);

/* crc.c */
extern int	crc(int, unsigned long *, unsigned long *);

/* devices.c */
extern DMenu	*deviceCreateMenu(DMenu *menu, DeviceType type, int (*hook)(dialogMenuItem *d),
				  int (*check)(dialogMenuItem *d));
extern void	deviceGetAll(void);
extern Device	**deviceFind(char *name, DeviceType type);
extern int	deviceCount(Device **devs);
extern Device	*new_device(char *name);
extern Device	*deviceRegister(char *name, char *desc, char *devname, DeviceType type, Boolean enabled,
				Boolean (*init)(Device *mediadev), int (*get)(Device *dev, char *file, Boolean probe),
				Boolean (*close)(Device *mediadev, int fd), void (*shutDown)(Device *mediadev),
				void *private);
extern Boolean	dummyInit(Device *dev);
extern int	dummyGet(Device *dev, char *dist, Boolean probe);
extern Boolean	dummyClose(Device *dev, int fd);
extern void	dummyShutdown(Device *dev);

/* disks.c */
extern int	diskPartitionEditor(dialogMenuItem *self);
extern int	diskPartitionWrite(dialogMenuItem *self);
extern void	diskPartition(Device *dev, Disk *d);

/* dispatch.c */
extern int	dispatchCommand(char *command);

/* dist.c */
extern int	distReset(dialogMenuItem *self);
extern int	distSetCustom(char *str);
extern int	distSetDeveloper(dialogMenuItem *self);
extern int	distSetXDeveloper(dialogMenuItem *self);
extern int	distSetKernDeveloper(dialogMenuItem *self);
extern int	distSetUser(dialogMenuItem *self);
extern int	distSetXUser(dialogMenuItem *self);
extern int	distSetMinimum(dialogMenuItem *self);
extern int	distSetEverything(dialogMenuItem *self);
extern int	distSetDES(dialogMenuItem *self);
extern int	distSetSrc(dialogMenuItem *self);
extern int	distSetXF86(dialogMenuItem *self);
extern int	distExtractAll(dialogMenuItem *self);

/* dmenu.c */
extern int	dmenuDisplayFile(dialogMenuItem *tmp);
extern int	dmenuSubmenu(dialogMenuItem *tmp);
extern int	dmenuSystemCommand(dialogMenuItem *tmp);
extern int	dmenuSystemCommandBox(dialogMenuItem *tmp);
extern int	dmenuExit(dialogMenuItem *tmp);
extern int	dmenuSetVariable(dialogMenuItem *tmp);
extern int	dmenuToggleVariable(dialogMenuItem *tmp);
extern int	dmenuSetFlag(dialogMenuItem *tmp);
extern int	dmenuSetValue(dialogMenuItem *tmp);
extern Boolean	dmenuOpen(DMenu *menu, int *choice, int *scroll, int *curr, int *max);
extern Boolean	dmenuOpenSimple(DMenu *menu);
extern int	dmenuVarCheck(dialogMenuItem *item);
extern int	dmenuFlagCheck(dialogMenuItem *item);
extern int	dmenuRadioCheck(dialogMenuItem *item);

/* doc.c */
extern int	docBrowser(dialogMenuItem *self);
extern int	docShowDocument(dialogMenuItem *self);

/* dos.c */
extern Boolean	mediaInitDOS(Device *dev);
extern int	mediaGetDOS(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownDOS(Device *dev);

/* floppy.c */
extern int	getRootFloppy(void);
extern Boolean	mediaInitFloppy(Device *dev);
extern int	mediaGetFloppy(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownFloppy(Device *dev);

/* ftp_strat.c */
extern Boolean	mediaCloseFTP(Device *dev, int fd);
extern Boolean	mediaInitFTP(Device *dev);
extern int	mediaGetFTP(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownFTP(Device *dev);

/* globals.c */
extern void	globalsInit(void);

/* index.c */
int		index_get(char *fname, PkgNodePtr papa);
int		index_read(int fd, PkgNodePtr papa);
int		index_menu(PkgNodePtr top, PkgNodePtr plist, int *pos, int *scroll);
void		index_init(PkgNodePtr top, PkgNodePtr plist);
void		index_node_free(PkgNodePtr top, PkgNodePtr plist);
void		index_sort(PkgNodePtr top);
void		index_print(PkgNodePtr top, int level);
int		index_extract(Device *dev, PkgNodePtr top, PkgNodePtr plist);

/* install.c */
extern int	installCommit(dialogMenuItem *self);
extern int	installCustomCommit(dialogMenuItem *self);
extern int	installExpress(dialogMenuItem *self);
extern int	installNovice(dialogMenuItem *self);
extern int	installFixitCDROM(dialogMenuItem *self);
extern int	installFixitFloppy(dialogMenuItem *self);
extern int	installFixup(dialogMenuItem *self);
extern int	installUpgrade(dialogMenuItem *self);
extern int	installFilesystems(dialogMenuItem *self);
extern int	installVarDefaults(dialogMenuItem *self);
extern Boolean	copySelf(void);

/* installFinal.c */
extern int	configGated(dialogMenuItem *self);
extern int	configSamba(dialogMenuItem *self);
extern int	configPCNFSD(dialogMenuItem *self);
extern int	configNFSServer(dialogMenuItem *self);

/* label.c */
extern int	diskLabelEditor(dialogMenuItem *self);
extern int	diskLabelCommit(dialogMenuItem *self);

/* lndir.c */
extern int	lndir(char *from, char *to);

/* makedevs.c (auto-generated) */
extern const char	termcap_vt100[];
extern const char	termcap_cons25[];
extern const char	termcap_cons25_m[];
extern const char	termcap_cons25r[];
extern const char	termcap_cons25r_m[];
extern const char	termcap_cons25l1[];
extern const char	termcap_cons25l1_m[];
extern const u_char	font_iso_8x16[];
extern const u_char	font_cp850_8x16[];
extern const u_char	font_cp866_8x16[];
extern const u_char	koi8_r2cp866[];
extern u_char		default_scrnmap[];

/* media.c */
extern char	*cpioVerbosity(void);
extern int	mediaSetCDROM(dialogMenuItem *self);
extern int	mediaSetFloppy(dialogMenuItem *self);
extern int	mediaSetDOS(dialogMenuItem *self);
extern int	mediaSetTape(dialogMenuItem *self);
extern int	mediaSetFTP(dialogMenuItem *self);
extern int	mediaSetFTPActive(dialogMenuItem *self);
extern int	mediaSetFTPPassive(dialogMenuItem *self);
extern int	mediaSetUFS(dialogMenuItem *self);
extern int	mediaSetNFS(dialogMenuItem *self);
extern int	mediaSetFtpOnError(dialogMenuItem *self);
extern int	mediaSetFtpUserPass(dialogMenuItem *self);
extern int	mediaSetCPIOVerbosity(dialogMenuItem *self);
extern int	mediaGetType(dialogMenuItem *self);
extern Boolean	mediaExtractDist(char *dir, int fd);
extern Boolean	mediaExtractDistBegin(char *dir, int *fd, int *zpid, int *cpic);
extern Boolean	mediaExtractDistEnd(int zpid, int cpid);
extern Boolean	mediaVerify(void);

/* misc.c */
extern Boolean	file_readable(char *fname);
extern Boolean	file_executable(char *fname);
extern Boolean	directory_exists(const char *dirname);
extern char	*string_concat(char *p1, char *p2);
extern char	*string_concat3(char *p1, char *p2, char *p3);
extern char	*string_prune(char *str);
extern char	*string_skipwhite(char *str);
extern char	*string_copy(char *s1, char *s2);
extern char	*pathBaseName(const char *path);
extern void	safe_free(void *ptr);
extern void	*safe_malloc(size_t size);
extern void	*safe_realloc(void *orig, size_t size);
extern dialogMenuItem *item_add(dialogMenuItem *list, char *prompt, char *title, 
				int (*checked)(dialogMenuItem *self),
				int (*fire)(dialogMenuItem *self),
				void (*selected)(dialogMenuItem *self, int is_selected),
				void *data, int aux, int *curr, int *max);
extern void	items_free(dialogMenuItem *list, int *curr, int *max);
extern int	Mkdir(char *, void *data);
extern int	Mount(char *, void *data);
extern WINDOW	*savescr(void);
extern void	restorescr(WINDOW *w);

/* msg.c */
extern Boolean	isDebug(void);
extern void	msgInfo(char *fmt, ...);
extern void	msgYap(char *fmt, ...);
extern void	msgWarn(char *fmt, ...);
extern void	msgDebug(char *fmt, ...);
extern void	msgError(char *fmt, ...);
extern void	msgFatal(char *fmt, ...);
extern void	msgConfirm(char *fmt, ...);
extern void	msgNotify(char *fmt, ...);
extern void	msgWeHaveOutput(char *fmt, ...);
extern int	msgYesNo(char *fmt, ...);
extern char	*msgGetInput(char *buf, char *fmt, ...);
extern int	msgSimpleConfirm(char *);
extern int	msgSimpleNotify(char *);

/* network.c */
extern Boolean	mediaInitNetwork(Device *dev);
extern void	mediaShutdownNetwork(Device *dev);

/* nfs.c */
extern Boolean	mediaInitNFS(Device *dev);
extern int	mediaGetNFS(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownNFS(Device *dev);

/* options.c */
extern int	optionsEditor(dialogMenuItem *self);

/* package.c */
extern int	package_add(char *name);
extern int	package_extract(Device *dev, char *name, Boolean depended);
extern Boolean	package_exists(char *name);

/* system.c */
extern void	systemInitialize(int argc, char **argv);
extern void	systemShutdown(int status);
extern int	execExecute(char *cmd, char *name);
extern int	systemExecute(char *cmd);
extern int	systemDisplayHelp(char *file);
extern char	*systemHelpFile(char *file, char *buf);
extern void	systemChangeFont(const u_char font[]);
extern void	systemChangeLang(char *lang);
extern void	systemChangeTerminal(char *color, const u_char c_termcap[], char *mono, const u_char m_termcap[]);
extern void	systemChangeScreenmap(const u_char newmap[]);
extern void	systemCreateHoloshell(void);
extern int	vsystem(char *fmt, ...);

/* tape.c */
extern char	*mediaTapeBlocksize(void);
extern Boolean	mediaInitTape(Device *dev);
extern int	mediaGetTape(Device *dev, char *file, Boolean probe);
extern void	mediaShutdownTape(Device *dev);

/* tcpip.c */
extern int	tcpOpenDialog(Device *dev);
extern int	tcpMenuSelect(dialogMenuItem *self);
extern int	tcpInstallDevice(char *str);
extern Boolean	tcpDeviceSelect(void);

/* termcap.c */
extern int	set_termcap(void);

/* ufs.c */
extern void	mediaShutdownUFS(Device *dev);
extern Boolean	mediaInitUFS(Device *dev);
extern int	mediaGetUFS(Device *dev, char *file, Boolean probe);

/* variable.c */
extern void	variable_set(char *var);
extern void	variable_set2(char *name, char *value);
extern char 	*variable_get(char *var);
extern void	variable_unset(char *var);
extern char	*variable_get_value(char *var, char *prompt);

/* wizard.c */
extern void	slice_wizard(Disk *d);

#endif
/* _SYSINSTALL_H_INCLUDE */
