################################################################################
#                                    RULES                                     #
################################################################################

# ==================================> Header <================================ #

header:
	@if [ $(S) -ne 0 ]; then \
		:; \
	else \
		printf "%b" "$(GREEN)"; \
		echo "           ___  _____ ___  ___      _        "; \
		echo "          /   |/ __  \|  \/  |     | |       "; \
		echo "         / /| |\`' / /'| .  . | __ _| | _____ "; \
		echo "        / /_| |  / /  | |\/| |/ _\` | |/ / _ \\"; \
		echo "        \___  |./ /___| |  | | (_| |   <  __/"; \
		echo "            |_/\_____/\_|  |_/\__,_|_|\_\___|"; \
		printf "%43b" "(run with \"V=1\" for Verbose)\n"; \
		printf "%38b" "(\"N=1\" for Normal)\n"; \
		printf "%42b" "(\"S=1\" for Silent)$(NO_COLOR)\n\n"; \
		printf "%b" "$(BLUE)Name:	$(GREEN)$(NAME)\n"; \
		printf "%b" "$(BLUE)Author:	$(GREEN)$(AUTHOR)$(NO_COLOR)\n\n"; \
	fi

# =============================> Special Targets <============================ #

.PHONY:			header

################################################################################
#                                    VISUALS                                   #
################################################################################

# =================================> Colors <================================= #

RED			=	\033[0;31m
B_RED		=	\033[0;91m
GREEN		=	\033[0;32m
B_GREEN		=	\033[0;92m
YELLOW		=	\033[0;33m
B_YELLOW	=	\033[0;93m
BLUE		=	\033[0;34m
B_BLUE		=	\033[0;94m
MAGENTA		=	\033[0;35m
B_MAGENTA	=	\033[0;95m
CYAN		=	\033[0;36m
B_CYAN		=	\033[0;96m
GRAY		=	\033[0;90m
B_GRAY		=	\033[0;37m
NO_COLOR	=	\033[m

# =================================> Strings <================================ #

MKDIR_MSG	=	Creating directory
LN_MSG		=	Creating softlink
CP_MSG		=	Copying directory
AR_MSG		=	Creating/Updating
PROCESS_MSG	=	Processing
FLAGS_MSG	=	Compiler flags
FLAGS		=	$(CXXFLAGS) $(CPPFLAGS) $(LDDFLAGS)
COMPIL_MSG	=	Compiling & assembling
LINK_MSG	=	Linking
MAKE_MSG	=	Calling Make in
CHMOD_MSG	=	Setting permission

# ===============================> Parameters <=============================== #

# Verbose / Normal / Silent / show compilation Flags
V			=	0
N			=	0
S			=	0
F			=	1

# ================================> Functions <=============================== #

ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(detected_OS),Darwin)
	RUN_CMD = script -q "$(@F).log" $(1) > /dev/null; \
				RESULT=$$? ; \
				sed -i "" -e "s/\^D//g" -e $$"s/\x08//g" -e "/^$$/d" "$(@F).log"
else ifeq ($(detected_OS),Linux)
	RUN_CMD = script -q -e -c "$(1)" "$(@F).log" > /dev/null; \
				RESULT=$$?; \
				sed -i '1d' "$(@F).log"; \
				sed -i "$$(($$(wc -l < "$(@F).log")-1)),\$$d" "$(@F).log"
else
	RUN_CMD = $(1) 2> "$(@F).log"; \
				RESULT=$$?
endif

define run
if [ $(S) -ne 0 ]; then \
	:; \
elif [ $(N) -ne 0 ]; then \
	printf "%b\n" "$(1)"; \
elif [ $(V) -ne 0 ]; then \
	printf "%b\n" "$(GRAY)$(1)"; \
elif ( echo "$(@F)" | grep -q '\.o' ) && [ $(F) -ne 0 ]; then \
	printf "%-30b%-40b\n" "$(BLUE)$(FLAGS_MSG)" "$(GRAY)$(FLAGS)$(NO_COLOR)"; \
fi
if [ $(S) -ne 0 ]; then \
	:; \
elif ( echo "$(2)" | grep -q 'clean' ) && [ $(N) -eq 0 ]; then \
	printf "%-63b" "$(BLUE)$(2)$(NO_COLOR)"; \
elif ( echo "$(2)" | grep -q 'Make' ) && [ $(N) -eq 0 ]; then \
	printf "%-30b%-40b" "$(BLUE)$(2)" "$(B_MAGENTA)$(3)$(NO_COLOR)"; \
elif [ $(N) -eq 0 ]; then \
	printf "%-30b%-40b" "$(BLUE)$(2)" "$(3)$(@)$(NO_COLOR)"; \
fi
if ( echo "$(2)" | grep -q 'clean' ); then \
	$(1); \
	RESULT=$$?; \
elif ( echo "$(2)" | grep -q 'Make' ); then \
	printf "\n"; \
	$(1); \
	exit $$?; \
else \
	$(RUN_CMD); \
fi ; \
if [ $(S) -ne 0 ]; then \
	:; \
elif [ $(N) -eq 0 -a $$RESULT -ne 0 ]; then \
	printf "%b\n" " $(RED)[KO]"; \
elif [ $(N) -eq 0 ] && ( echo "$(@F)" | grep -q 'test' ) \
		&& ! ( cat "$(@F).log" 2> /dev/null | grep -q 'FAILED' ); then \
	printf "%b\n" " $(GREEN)[OK]"; \
elif [ $(N) -eq 0 -a -s "$(@F).log" ]; then \
	printf "%b\n" " $(YELLOW)[WARN]"; \
elif [ $(N) -eq 0 ]; then  \
	printf "%b\n" " $(GREEN)[OK]"; \
fi; \
printf "%b" "$(NO_COLOR)"; \
cat "$(@F).log" 2> /dev/null; \
rm -f "$(@F).log"; \
exit $$RESULT
endef
