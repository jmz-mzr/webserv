#>	DISPLAY
SHADOW		=	$(shell printf '─%.0s' {0..21})
SEP1		=	$(shell printf '$(PAD_CHAR3)%.0s' {0..41})
SEP2		=	$(shell printf '⸻%.0s' {1..36})

#>	FUNCTIONS
VARS		=	$(foreach var,$(.VARIABLES),$(info $(var) = $($(var))))
rwildcard	=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

define print_title =
$(eval PAD_LEN = $(shell NAME=$(1) ;  echo "x ="$${#NAME}" ; ((21 - x) / 2)" | bc))
@printf '\n\n'
@printf '%*s$(FWHT)╔═════════════════════╗$(RESET)\n' 42 ''
@printf '$(SEP1)$(FWHT)║%*s$(WHT)$(1)%*s$(FWHT)╟─┐$(RESET)\n' $(PAD_LEN) '' $(shell NAME=$(1) ; echo "if (!("$${#NAME}" % 2)) $(PAD_LEN) + 1 else $(PAD_LEN)" | bc) ''
@printf '%*s$(FWHT)╚╤════════════════════╝ │$(RESET)\n' 42 ''
@printf '%*s $(FWHT)└$(SHADOW)┘$(RESET)\n' 42 ''
endef

define print_str_format_width =
echo -e '$(shell read -a array <<< "$(1)";curr="";for line in $${array[@]}; do if (( $${#curr} > $(2) )); then echo "\t\t$${curr}\n";curr="";else curr+="$${line} ";fi;done;if [[ -n $${curr} ]]; then echo "\t\t$${curr}\n";fi)'
endef

#> 🡆🡆⮞⮞

#>	TERMCAPS
UP			=	\033[1A
DELETE		=	\033[2K
DELPREV		=	$(UP)\r$(DELETE)

#>	EMOJIS
CHECK		=	\xE2\x9C\x94
CROSS		=	\xE2\x9D\x8C
TRASH		=	\xF0\x9F\x97\x91

#>	BOX DRAWING
UP_L		=	▛
UP_R		=	▜
DOWN_L		=	▙
DOWN_R		=	▟
PAD_CHAR1	=	▓
PAD_CHAR2	=	▒
PAD_CHAR3	=	░
RVERT		=	▐
LVERT		=	▌
UHLINE		=	▄
DHLINE		=	▀

REVERSE		=	\033[7m

#>	Reset
RESET		=	\033[0m

#>  Regular
BLK			=	\033[0;30m
RED			=	\033[0;31m
GRN			=	\033[0;32m
YEL			=	\033[0;33m
BLU			=	\033[0;34m
MAG			=	\033[0;35m
CYN			=	\033[0;36m
WHT			=	\033[0;37m

#>	Bold
BBLK		=	\033[1;30m
BRED		=	\033[1;31m
BGRN		=	\033[1;32m
BYEL		=	\033[1;33m
BBLU		=	\033[1;34m
BMAG		=	\033[1;35m
BCYN		=	\033[1;36m
BWHT		=	\033[1;37m

#>	Faint
FBLK		=	\033[2;30m
FRED		=	\033[2;31m
FGRN		=	\033[2;32m
FYEL		=	\033[2;33m
FBLU		=	\033[2;34m
FMAG		=	\033[2;35m
FCYN		=	\033[2;36m
FWHT		=	\033[2;37m
FWHT2		=	\033[2;97m

#>  Underline
UBLK		=	\033[4;30m
URED		=	\033[4;31m
UGRN		=	\033[4;32m
UYEL		=	\033[4;33m
UBLU		=	\033[4;34m
UMAG		=	\033[4;35m
UCYN		=	\033[4;36m
UWHT		=	\033[4;37m

#>  Background
BLKB		=	\033[40m
REDB		=	\033[41m
GRNB		=	\033[42m
YELB		=	\033[43m
BLUB		=	\033[44m
MAGB		=	\033[45m
CYNB		=	\033[46m
WHTB		=	\033[47m

#>  High intensity background
BLKHB		=	\033[0;100m
REDHB		=	\033[0;101m
GRNHB		=	\033[0;102m
YELHB		=	\033[0;103m
BLUHB		=	\033[0;104m
MAGHB		=	\033[0;105m
CYNHB		=	\033[0;106m
WHTHB		=	\033[0;107m

#>	High intensity foreground
HBLK		=	\033[0;90m
HRED		=	\033[0;91m
HGRN		=	\033[0;92m
HYEL		=	\033[0;93m
HBLU		=	\033[0;94m
HMAG		=	\033[0;95m
HCYN		=	\033[0;96m
HWHT		=	\033[0;97m

#>	Bold high intensity foreground
BHBLK		=	\033[1;90m
BHRED		=	\033[1;91m
BHGRN		=	\033[1;92m
BHYEL		=	\033[1;93m
BHBLU		=	\033[1;94m
BHMAG		=	\033[1;95m
BHCYN		=	\033[1;96m
BHWHT		=	\033[1;97m