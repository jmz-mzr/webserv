#!/bin/bash

WORKDIR=${0%/*}
WWWDIR=$WORKDIR/www
CONF_FILE="$WORKDIR/default.conf"
PLATFORM=$(uname -s)

if [[ "$PLATFORM" -eq "Linux" ]]; then
	sed -i 's/macos/ubuntu/g' "$CONF_FILE"
elif [[ "$PLATFORM" -eq "Darwin" ]]; then
	sed -i "" -e 's/ubuntu/macos/g' "$CONF_FILE"
fi

chmod +x $WWWDIR/webserv.42.fr/{index.php,cgi-bin/info.php}
chmod -R +x $WWWDIR/webserv{.42,}.test/cgi-bin/

# if [ -d "$HOME/goinfre" -a -w "$HOME/goinfre" ]; then
# 	export PREFIX="$HOME/goinfre"
# else
# 	export PREFIX="/usr/local"
# fi