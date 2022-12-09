#>	DISPLAY
WIDTH		=	48
PAD1		=	$(shell printf '$(PAD_CHAR3)%.0s' {1..4})
PAD2		=	$(shell printf '$(PAD_CHAR2)%.0s' {1..9})
PAD3		=	$(shell printf '$(PAD_CHAR1)%.0s' {1..20})
PAD			=	$(PAD1)$(PAD2)$(PAD3)
HLINE		=	$(shell printf '═%.0s' {0..64})

define print_title =
@printf '\n$(PAD)$(WHTB)$(BBLK)%-*s$(1) $(RESET)╗\n╚$(HLINE)╝\n' $(shell NAME='$(1)'; echo "32 - "$${#NAME}"" | bc)
endef

#>	FUNCTIONS
rwildcard	=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
VARS		=	$(foreach var,$(.VARIABLES),$(info $(var) = $($(var))))

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

#>	Reset
RESET		=	\e[0m

#>  Regular
BLK			=	\e[0;30m
RED			=	\e[0;31m
GRN			=	\e[0;32m
YEL			=	\e[0;33m
BLU			=	\e[0;34m
MAG			=	\e[0;35m
CYN			=	\e[0;36m
WHT			=	\e[0;37m

#>	Bold
BBLK		=	\e[1;30m
BRED		=	\e[1;31m
BGRN		=	\e[1;32m
BYEL		=	\e[1;33m
BBLU		=	\e[1;34m
BMAG		=	\e[1;35m
BCYN		=	\e[1;36m
BWHT		=	\e[1;37m

#>	Faint
FBLK		=	\e[2;30m
FRED		=	\e[2;31m
FGRN		=	\e[2;32m
FYEL		=	\e[2;33m
FBLU		=	\e[2;34m
FMAG		=	\e[2;35m
FCYN		=	\e[2;36m
FWHT		=	\e[2;37m
FWHT2		=	\e[2;97m

#>  Underline
UBLK		=	\e[4;30m
URED		=	\e[4;31m
UGRN		=	\e[4;32m
UYEL		=	\e[4;33m
UBLU		=	\e[4;34m
UMAG		=	\e[4;35m
UCYN		=	\e[4;36m
UWHT		=	\e[4;37m

#>  Background
BLKB		=	\e[40m
REDB		=	\e[41m
GRNB		=	\e[42m
YELB		=	\e[43m
BLUB		=	\e[44m
MAGB		=	\e[45m
CYNB		=	\e[46m
WHTB		=	\e[47m

#>  High intensity background
BLKHB		=	\e[0;100m
REDHB		=	\e[0;101m
GRNHB		=	\e[0;102m
YELHB		=	\e[0;103m
BLUHB		=	\e[0;104m
MAGHB		=	\e[0;105m
CYNHB		=	\e[0;106m
WHTHB		=	\e[0;107m

#>	High intensity foreground
HBLK		=	\e[0;90m
HRED		=	\e[0;91m
HGRN		=	\e[0;92m
HYEL		=	\e[0;93m
HBLU		=	\e[0;94m
HMAG		=	\e[0;95m
HCYN		=	\e[0;96m
HWHT		=	\e[0;97m

#>	Bold high intensity foreground
BHBLK		=	\e[1;90m
BHRED		=	\e[1;91m
BHGRN		=	\e[1;92m
BHYEL		=	\e[1;93m
BHBLU		=	\e[1;94m
BHMAG		=	\e[1;95m
BHCYN		=	\e[1;96m
BHWHT		=	\e[1;97m