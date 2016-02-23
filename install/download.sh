#!/bin/sh
# Download utility to automatically select one of the installed downloader and
# use it to download from a given url to a given target
# If a third parameter is provided, it is a md5 checksum which is checked
# against the received file with the command md5sum

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
	if $dl $SOURCE > $TARGET ; then
	    return 0
	else
	    return 1
	fi
    fi

    dl=`which wget`
    if [ $? -eq 0 ] ; then
	if $dl $SOURCE -O $TARGET ; then
	    return 0
	else
	    return 0
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

    sum=`$ck "$TARGET" | cut -d' ' -f1`
    if [ $sum != $MD5SUM ] ; then
	return 1
    fi

    return 0
}


download
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
