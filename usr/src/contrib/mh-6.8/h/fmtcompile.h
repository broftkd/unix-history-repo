/* @(#)$Id: fmtcompile.h,v 1.9 1992/02/09 07:12:48 jromine Exp $ */

/* Format Types */
/* -------------*/

/* types that output text */
#define FT_COMP		1	/* the text of a component */
#define FT_COMPF	2	/* comp text, filled */
#define FT_LIT		3	/* literal text */
#define FT_LITF		4	/* literal text, filled */
#define FT_CHAR		5	/* a single ascii character */
#define FT_NUM		6	/* "value" as decimal number */
#define FT_NUMF		7	/* "value" as filled dec number */
#define FT_STR		8	/* "str" as text */
#define FT_STRF		9	/* "str" as text, filled */
#define FT_STRFW	10	/* "str" as text, filled, width in "value" */
#define FT_PUTADDR	11	/* split and print address line */

/* types that modify the "str" or "value" registers */
#define FT_LS_COMP	12	/* set "str" to component text */
#define FT_LS_LIT	13	/* set "str" to literal text */
#define FT_LS_GETENV	14	/* set "str" to getenv(text) */
#define FT_LS_TRIM	15	/* trim trailing white space from "str" */
#define FT_LV_COMP	16	/* set "value" to comp (as dec. num) */
#define FT_LV_COMPFLAG	17	/* set "value" to comp flag word */
#define FT_LV_LIT	18	/* set "value" to literal num */
#define FT_LV_DAT	19	/* set "value" to dat[n] */
#define FT_LV_STRLEN	20	/* set "value" to length of "str" */
#define FT_LV_PLUS_L	21	/* set "value" += literal */
#define FT_LV_MINUS_L	22	/* set "value" -= literal */
#define FT_LV_DIVIDE_L	23	/* set "value" to value / literal */
#define	FT_LV_MODULO_L	24	/* set "value" to value % literal */
#define FT_LV_CHAR_LEFT 25      /* set "value" to char left in output */

#define FT_LS_MONTH     26      /* set "str" to tws month */
#define FT_LS_LMONTH    27      /* set "str" to long tws month */
#define FT_LS_ZONE      28      /* set "str" to tws timezone */
#define FT_LS_DAY       29      /* set "str" to tws weekday */
#define FT_LS_WEEKDAY   30      /* set "str" to long tws weekday */
#define FT_LS_822DATE   31      /* set "str" to 822 date str */
#define FT_LS_PRETTY    32      /* set "str" to pretty (?) date str */
#define FT_LV_SEC       33      /* set "value" to tws second */
#define FT_LV_MIN       34      /* set "value" to tws minute */
#define FT_LV_HOUR      35      /* set "value" to tws hour */
#define FT_LV_MDAY      36      /* set "value" to tws day of month */
#define FT_LV_MON       37      /* set "value" to tws month */
#define FT_LV_YEAR      38      /* set "value" to tws year */
#define FT_LV_YDAY      39      /* set "value" to tws day of year */
#define FT_LV_WDAY      40      /* set "value" to tws weekday */
#define FT_LV_ZONE      41      /* set "value" to tws timezone */
#define FT_LV_CLOCK     42      /* set "value" to tws clock */
#define FT_LV_RCLOCK    43      /* set "value" to now - tws clock */
#define FT_LV_DAYF      44      /* set "value" to tws day flag */
#define FT_LV_DST       45      /* set "value" to tws daylight savings flag */
#define FT_LV_ZONEF     46      /* set "value" to tws timezone flag */

#define FT_LS_PERS      47      /* set "str" to person part of addr */
#define FT_LS_MBOX      48      /* set "str" to mbox part of addr */
#define FT_LS_HOST      49      /* set "str" to host part of addr */
#define FT_LS_PATH      50      /* set "str" to route part of addr */
#define FT_LS_GNAME     51      /* set "str" to group part of addr */
#define FT_LS_NOTE      52      /* set "str" to comment part of addr */
#define FT_LS_ADDR      53      /* set "str" to mbox@host */
#define FT_LS_822ADDR   54      /* set "str" to 822 format addr */
#define FT_LS_FRIENDLY  55      /* set "str" to "friendly" format addr */
#define FT_LV_HOSTTYPE  56      /* set "value" to addr host type */
#define FT_LV_INGRPF    57      /* set "value" to addr in-group flag */
#define FT_LV_NOHOSTF   58      /* set "value" to addr no-host flag */

/* Date Coercion */
#define FT_LOCALDATE    59      /* Coerce date to local timezone */
#define FT_GMTDATE      60      /* Coerce date to gmt */

/* pre-format processing */
#define FT_PARSEDATE    61      /* parse comp into a date (tws) struct */
#define FT_PARSEADDR    62      /* parse comp into a mailaddr struct */
#define FT_FORMATADDR   63      /* let external routine format addr */
#define FT_MYMBOX       64      /* do "mymbox" test on comp */

/* misc. */		/* ADDTOSEQ only works if you include "options LBL" */
#define FT_ADDTOSEQ     65      /* add current msg to a sequence */

/* conditionals & control flow (must be last) */
#define FT_SAVESTR      66      /* save current str reg */
#define FT_DONE         67      /* stop formatting */
#define FT_PAUSE        68      /* pause */
#define FT_NOP          69      /* nop */
#define FT_GOTO         70      /* (relative) goto */
#define FT_IF_S_NULL    71      /* test if "str" null */
#define FT_IF_S         72      /* test if "str" non-null */
#define FT_IF_V_EQ      73      /* test if "value" = literal */
#define FT_IF_V_NE      74      /* test if "value" != literal */
#define FT_IF_V_GT      75      /* test if "value" > literal */
#define FT_IF_MATCH     76      /* test if "str" contains literal */
#define FT_IF_AMATCH    77      /* test if "str" starts with literal */
#define FT_S_NULL       78      /* V = 1 if "str" null */
#define FT_S_NONNULL    79      /* V = 1 if "str" non-null */
#define FT_V_EQ         80      /* V = 1 if "value" = literal */
#define FT_V_NE         81      /* V = 1 if "value" != literal */
#define FT_V_GT         82      /* V = 1 if "value" > literal */
#define FT_V_MATCH      83      /* V = 1 if "str" contains literal */
#define FT_V_AMATCH     84      /* V = 1 if "str" starts with literal */

#define IF_FUNCS FT_S_NULL      /* start of "if" functions */
