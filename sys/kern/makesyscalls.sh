#! /bin/sh -
#	@(#)makesyscalls.sh	8.1 (Berkeley) 6/10/93
# $FreeBSD$

set -e

# name of compat option:
compat=COMPAT_43

# output files:
sysnames="syscalls.c"
sysproto="../sys/sysproto.h"
sysproto_h=_SYS_SYSPROTO_H_
syshdr="../sys/syscall.h"
syssw="init_sysent.c"
syshide="../sys/syscall-hide.h"
syscallprefix="SYS_"
switchname="sysent"
namesname="syscallnames"

# tmp files:
sysdcl="sysent.dcl"
syscompat="sysent.compat"
syscompatdcl="sysent.compatdcl"
sysent="sysent.switch"
sysinc="sysinc.switch"
sysarg="sysarg.switch"

trap "rm $sysdcl $syscompat $syscompatdcl $sysent $sysinc $sysarg" 0

case $# in
    0)	echo "Usage: $0 input-file <config-file>" 1>&2
	exit 1
	;;
esac

if [ -f $2 ]; then
	. $2
fi

sed -e '
s/\$//g
:join
	/\\$/{a\

	N
	s/\\\n//
	b join
	}
2,${
	/^#/!s/\([{}()*,]\)/ \1 /g
}
' < $1 | awk "
	BEGIN {
		sysdcl = \"$sysdcl\"
		sysproto = \"$sysproto\"
		sysproto_h = \"$sysproto_h\"
		syscompat = \"$syscompat\"
		syscompatdcl = \"$syscompatdcl\"
		sysent = \"$sysent\"
		sysinc = \"$sysinc\"
		sysarg = \"$sysarg\"
		sysnames = \"$sysnames\"
		syshdr = \"$syshdr\"
		compat = \"$compat\"
		syshide = \"$syshide\"
		syscallprefix = \"$syscallprefix\"
		switchname = \"$switchname\"
		namesname = \"$namesname\"
		infile = \"$1\"
		"'

		printf "/*\n * System call switch table.\n *\n" > sysinc
		printf " * DO NOT EDIT-- this file is automatically generated.\n" > sysinc

		printf "/*\n * System call prototypes.\n *\n" > sysarg
		printf " * DO NOT EDIT-- this file is automatically generated.\n" > sysarg

		printf "\n#ifdef %s\n\n", compat > syscompat

		printf "/*\n * System call names.\n *\n" > sysnames
		printf " * DO NOT EDIT-- this file is automatically generated.\n" > sysnames

		printf "/*\n * System call numbers.\n *\n" > syshdr
		printf " * DO NOT EDIT-- this file is automatically generated.\n" > syshdr
		printf "/*\n * System call hiders.\n *\n" > syshide
		printf " * DO NOT EDIT-- this file is automatically generated.\n" > syshide
	}
	NR == 1 {
		gsub("[$]FreeBSD: ", "", $0)
		gsub(" [$]", "", $0)

		printf " * created from%s\n */\n\n", $0 > sysinc

		printf "\n#ifdef %s\n", compat > sysent
		printf "#define compat(n, name) n, (sy_call_t *)__CONCAT(o,name)\n" > sysent
		printf("#else\n") > sysent
		printf("#define compat(n, name) 0, (sy_call_t *)nosys\n") > sysent
		printf("#endif\n\n") > sysent
		printf("/* The casts are bogus but will do for now. */\n") > sysent
		printf "struct sysent %s[] = {\n",switchname > sysent

		printf " * created from%s\n */\n\n", $0 > sysarg
		printf("#ifndef %s\n", sysproto_h) > sysarg
		printf("#define\t%s\n\n", sysproto_h) > sysarg
		printf "#include <sys/types.h>\n", $0 > sysarg
		printf "#include <sys/param.h>\n", $0 > sysarg
		printf "#include <sys/mount.h>\n\n", $0 > sysarg

		printf " * created from%s\n */\n\n", $0 > sysnames
		printf "char *%s[] = {\n", namesname > sysnames

		printf " * created from%s\n */\n\n", $0 > syshdr

		printf " * created from%s\n */\n\n", $0 > syshide
		next
	}
	NF == 0 || $1 ~ /^;/ {
		next
	}
	$1 ~ /^#[ 	]*include/ {
		print > sysinc
		next
	}
	$1 ~ /^#[ 	]*if/ {
		print > sysent
		print > sysdcl
		print > sysarg
		print > syscompat
		print > sysnames
		print > syshide
		savesyscall = syscall
		next
	}
	$1 ~ /^#[ 	]*else/ {
		print > sysent
		print > sysdcl
		print > sysarg
		print > syscompat
		print > sysnames
		print > syshide
		syscall = savesyscall
		next
	}
	$1 ~ /^#/ {
		print > sysent
		print > sysdcl
		print > sysarg
		print > syscompat
		print > sysnames
		print > syshide
		next
	}
	syscall != $1 {
		printf "%s: line %d: syscall number out of sync at %d\n", \
		   infile, NR, syscall
		printf "line is:\n"
		print
		exit 1
	}
	function parserr(was, wanted) {
		printf "%s: line %d: unexpected %s (expected %s)\n", \
		    infile, NR, was, wanted
		exit 1
	}
	function parseline() {
		f=4			# toss number and type
		argc= 0;
		bigargc = 0;
		if ($NF != "}") {
			funcalias=$(NF-2)
			argalias=$(NF-1)
			rettype=$NF
			end=NF-3
		} else {
			funcalias=""
			argalias=""
			rettype="int"
			end=NF
		}
		if ($2 == "NODEF") {
			funcname=$4
			return
		}
		if ($f != "{")
			parserr($f, "{")
		f++
		if ($end != "}")
			parserr($end, "}")
		end--
		if ($end != ";")
			parserr($end, ";")
		end--
		if ($end != ")")
			parserr($end, ")")
		end--

		f++	#function return type

		funcname=$f
		if (funcalias == "")
			funcalias = funcname
		if (argalias == "") {
			argalias = funcname "_args"
			if ($2 == "COMPAT")
				argalias = "o" argalias
		}
		f++

		if ($f != "(")
			parserr($f, ")")
		f++

		if (f == end) {
			if ($f != "void")
				parserr($f, "argument definition")
			return
		}

		while (f <= end) {
			argc++
			argtype[argc]=""
			oldf=""
			while (f < end && $(f+1) != ",") {
				if (argtype[argc] != "" && oldf != "*")
					argtype[argc] = argtype[argc]" ";
				argtype[argc] = argtype[argc]$f;
				oldf = $f;
				f++
			}
			if (argtype[argc] == "")
				parserr($f, "argument definition")
			if (argtype[argc] == "off_t")
				bigargc++
			argname[argc]=$f;
			f += 2;			# skip name, and any comma
		}
	}
	{	comment = $4
		if (NF < 7)
			for (i = 5; i <= NF; i++)
				comment = comment " " $i
	}
	$2 == "STD" || $2 == "NODEF" || $2 == "NOARGS"  || $2 == "NOPROTO" {
		parseline()
		if ((!nosys || funcname != "nosys") && \
		    (funcname != "lkmnosys")) {
			if (argc != 0 && $2 != "NOARGS" && $2 != "NOPROTO") {
				printf("struct\t%s {\n", argalias) > sysarg
				for (i = 1; i <= argc; i++)
					printf("\t%s %s;\n", argtype[i],
					    argname[i]) > sysarg
				printf("};\n") > sysarg
			}
			else if($2 != "NOARGS" && $2 != "NOPROTO")
				printf("struct\t%s {\n\tint dummy;\n};\n", \
					argalias) > sysarg
		}
		if ($2 != "NOPROTO" && (!nosys || funcname != "nosys") && \
		    (!lkmnosys || funcname != "lkmnosys")) {
			printf("%s\t%s __P((struct proc *, struct %s *, int []))", \
			    rettype, funcname, argalias) > sysdcl
			if (funcname == "exit")
				printf(" __dead2") > sysdcl
			printf(";\n") > sysdcl
		}
		if (funcname == "nosys")
			nosys = 1
		if (funcname == "lkmnosys")
			lkmnosys = 1
		printf("\t{ %d, (sy_call_t *)%s },\t\t", \
		    argc+bigargc, funcname) > sysent
		if(length(funcname) < 11)
			printf("\t") > sysent
		printf("/* %d = %s */\n", syscall, funcalias) > sysent
		printf("\t\"%s\",\t\t\t/* %d = %s */\n", \
		    funcalias, syscall, funcalias) > sysnames
		if ($2 != "NODEF")
			printf("#define\t%s%s\t%d\n", syscallprefix, \
		    	    funcalias, syscall) > syshdr
		if ($3 != "NOHIDE")
			printf("HIDE_%s(%s)\n", $3, funcname) > syshide
		syscall++
		next
	}
	$2 == "COMPAT" || $2 == "CPT_NOA" {
		parseline()
		if (argc != 0 && $2 != "CPT_NOA") {
			printf("struct\t%s {\n", argalias) > syscompat
			for (i = 1; i <= argc; i++)
				printf("\t%s %s;\n", argtype[i],
				    argname[i]) > syscompat
			printf("};\n") > syscompat
		}
		else if($2 != "CPT_NOA")
			printf("struct\t%s {\n\tint dummy;\n};\n", \
				argalias) > sysarg
		printf("%s\to%s __P((struct proc *, struct %s *, int []));\n", \
		    rettype, funcname, argalias) > syscompatdcl
		printf("\t{ compat(%d,%s) },\t\t/* %d = old %s */\n", \
		    argc+bigargc, funcname, syscall, funcalias) > sysent
		printf("\t\"old.%s\",\t\t/* %d = old %s */\n", \
		    funcalias, syscall, funcalias) > sysnames
		printf("\t\t\t\t/* %d is old %s */\n", \
		    syscall, funcalias) > syshdr
		if ($3 != "NOHIDE")
			printf("HIDE_%s(%s)\n", $3, funcname) > syshide
		syscall++
		next
	}
	$2 == "LIBCOMPAT" {
		parseline()
		printf("%s\to%s();\n", rettype, funcname) > syscompatdcl
		printf("\t{ compat(%d,%s) },\t\t/* %d = old %s */\n", \
		    argc+bigargc, funcname, syscall, funcalias) > sysent
		printf("\t\"old.%s\",\t\t/* %d = old %s */\n", \
		    funcalias, syscall, funcalias) > sysnames
		printf("#define\t%s%s\t%d\t/* compatibility; still used by libc */\n", \
		    syscallprefix, funcalias, syscall) > syshdr
		if ($3 != "NOHIDE")
			printf("HIDE_%s(%s)\n", $3, funcname) > syshide
		syscall++
		next
	}
	$2 == "OBSOL" {
		printf("\t{ 0, (sy_call_t *)nosys },\t\t\t/* %d = obsolete %s */\n", \
		    syscall, comment) > sysent
		printf("\t\"obs_%s\",\t\t\t/* %d = obsolete %s */\n", \
		    $4, syscall, comment) > sysnames
		printf("\t\t\t\t/* %d is obsolete %s */\n", \
		    syscall, comment) > syshdr
		if ($3 != "NOHIDE")
			printf("HIDE_%s(%s)\n", $3, $4) > syshide
		syscall++
		next
	}
	$2 == "UNIMPL" {
		printf("\t{ 0, (sy_call_t *)nosys },\t\t\t/* %d = %s */\n", \
		    syscall, comment) > sysent
		printf("\t\"#%d\",\t\t\t/* %d = %s */\n", \
		    syscall, syscall, comment) > sysnames
		if ($3 != "NOHIDE")
			printf("HIDE_%s(%s)\n", $3, $4) > syshide
		syscall++
		next
	}
	{
		printf "%s: line %d: unrecognized keyword %s\n", infile, NR, $2
		exit 1
	}
	END {
		printf("\n#endif /* %s */\n", compat) > syscompatdcl
		printf("\n#endif /* !%s */\n", sysproto_h) > syscompatdcl

		printf("};\n") > sysent
		printf("};\n") > sysnames
		printf("#define\t%sMAXSYSCALL\t%d\n", syscallprefix, syscall) \
			> syshdr
	} '

cat $sysinc $sysent >$syssw
cat $sysarg $sysdcl $syscompat $syscompatdcl > $sysproto
