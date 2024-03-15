#!/bin/bash

if [[ ! "$WORKDIR" ]]; then
	echo "WORKDIR undefined"
	exit 1
fi

PLATFORM=$(uname -s)

print_header() {
	echo -e "\033[0;33m"
	echo "                  (       (                      (              (     ";
	echo " (  (          (  )\ )    )\ )          *   )    )\ )  *   )    )\ )  ";
	echo " )\))(   '(  ( )\(()/((  (()/((   (   \` )  /((  (()/(\` )  /((  (()/(  ";
	echo "((_)()\ ) )\ )((_)/(_))\  /(_))\  )\   ( )(_))\  /(_))( )(_))\  /(_)) ";
	echo "_(())\_)(|(_|(_)_(_))((_)(_))((_)((_) (_(_()|(_)(_)) (_(_()|(_)(_))   ";
	echo "\ \((_)/ / __| _ ) __| __| _ \ \ / /  |_   _| __/ __||_   _| __| _ \  ";
	echo " \ \/\/ /| _|| _ \__ \ _||   /\ V /     | | | _|\__ \  | | | _||   /  ";
	echo "  \_/\_/ |___|___/___/___|_|_\ \_/      |_| |___|___/  |_| |___|_|_\  ";
	echo "                                                                      ";
	echo -e "\033[m"
}

install_webserv() {
	which webserv &> /dev/null
	if [[ $? != 0 ]]; then
		make -sC "$WORKDIR" install
	fi
}

run_webserv() {
	webserv $WORKDIR/default.conf &> /dev/null &
}

kill_webserv() {
	ps | grep webserv &> /dev/null
	[[ "$?" ]] && pkill webserv
}

run_42_tester() {
	if [[ "$PLATFORM" == "Linux" ]]; then
		os=ubuntu
	elif [[ "$PLATFORM" == "Darwin" ]]; then
		os=macos
	else
		echo "$PLATFORM not supported"
		exit 1
	fi
	$WORKDIR/tests/42/"$os"_tester http://localhost:8081
}

options=($'\033[0;36mUnit tests\033[m' $'\033[0;36m42 tester\033[m' $'\033[0;36mPython tester\033[m' $'\033[0;90mQuit\033[m')
PS3=$'\n\033[1;37mSelect a tester to run:\033[m '

print_header
while true; do
	select opt in "${options[@]}"
	do
		case $opt in
			$'\033[0;36mUnit tests\033[m') install_webserv && $WORKDIR/webserv_test ; ret="$?" ; break ;;
			$'\033[0;36m42 tester\033[m') install_webserv && run_webserv && run_42_tester ; ret="$?" ; break ;;
			$'\033[0;36mPython tester\033[m') install_webserv && run_webserv && python3 "$WORKDIR/tests/http_py/main.py" ; ret="$?" ; break ;;
			$'\033[0;90mQuit\033[m') kill_webserv ; break 2 ;;
			*) echo "Invalid option $REPLY" ; break ;;
		esac
	done
	print_header
	[[ "$ret" -ne 0 ]] && echo -e "❌  $opt\t\033[0;31mYou n00b, you failed!!!\033[m\n" || echo -e "✅  $opt\t\033[0;32mYou're a roxXxor, GG!!!\033[m\n"
done

exit 0
