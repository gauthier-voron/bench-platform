#!/bin/sh
# Download utility to automatically select one of the installed downloader and
# use it to download from a given url to a given target
# If a third parameter is provided, it is a md5 checksum which is checked
# against the received file with the command md5sum

if [ "x$1" = "x-g" ] || [ "x$1" = "x--git" ] ; then
    GITDL=1
    shift
fi

SOURCE="$1" ; shift
TARGET="$1" ; shift
MD5SUM="$1" ; shift

if [ "x$SOURCE" = "x" ] ; then
    echo "$0: missing SOURCE parameter" >&2
    echo "Syntax: $0 SOURCE TARGET" >&2
elif [ "x$TARGET" = "x" ] ; then
    echo "$0: missing TARGET parameter" >&2
    echo "Syntax: $0 SOURCE TARGET" >&2
fi


download()
{
    dl=`which curl`
    if [ $? -eq 0 ] ; then
	if $dl -L "$SOURCE" > "$TARGET" ; then
	    return 0
	else
	    return 1
	fi
    fi

    dl=`which wget`
    if [ $? -eq 0 ] ; then
	if $dl "$SOURCE" -O "$TARGET" ; then
	    return 0
	else
	    return 1
	fi
    fi

    return 2
}

git_download()
{
    dl=`which git`
    if [ $? -eq 0 ] ; then
	export GIT_SSL_NO_VERIFY=1
	if $dl clone --progress "$SOURCE" "$TARGET" ; then
	    return 0
	else
	    return 1
	fi
    fi

    return 2
}

checksum()
{
    ck=`which md5sum`
    if [ $? -ne 0 ] ; then
	return 2
    fi

    if [ -f "$TARGET" ] ; then
	sum=`$ck "$TARGET" | cut -d' ' -f1`
    elif [ -d "$TARGET" ] ; then
	sum=`find "$TARGET" -type f | grep -v '.git' | sort \
	    | xargs $ck | $ck | cut -d' ' -f1`
    else
	return 2
    fi

    if [ $sum != $MD5SUM ] ; then
	return 1
    fi

    return 0
}


if [ "x$GITDL" != "x" ] ; then
    git_download
else
    download
fi
case $? in
    1)
	echo "$0: failed to download '$SOURCE'" >&2
	exit 1
	;;
    2)
	echo "$0: cannot find appropriate downloader" >&2
	echo "Please install either 'curl' or 'wget'" >&2
	exit 1
	;;
esac

if [ "x$MD5SUM" = "x" ] ; then
    exit 0
fi

checksum
case $? in
    1)
	echo "$0: bad checksum for '$SOURCE'" >&2
	exit 1
	;;
    2)
	echo "$0 cannot find appropriate digester" >&2
	echo "Please install 'md5sum'" >&2
	exit 1
	;;
esac

exit 0
