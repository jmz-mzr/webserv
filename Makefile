################################################################################
#                                    CONFIG                                    #
################################################################################

# =================================> Project <================================ #

NAME		=	webserv
AUTHOR		=	jmazoyer flohrel mtogbe
BUILD		?=	debug

ifneq ($(filter-out debug release,$(BUILD)),)
  $(error '$(BUILD)' is incorrect. Build options are 'debug' or 'release')
endif

ifneq ($(filter install, $(strip $(MAKECMDGOALS))),)
  override BUILD = release
endif

# ===============================> Directories <============================== #

SRCDIR		=	src
SUBDIR		=	config core utils
INCLDIR		=	include
BUILDIR		=	build/$(BUILD)
DEPDIR		=	$(BUILDIR)/.deps
TESTDIR		=	tests/cpp_gtest
WORKDIR		:=	$(realpath .)

ifeq ($(PREFIX),)
  PREFIX	=	/usr/local
endif

BINDIR		=	$(PREFIX)/bin
SYSCONFDIR	=	$(PREFIX)/etc
CONFDIR		=	$(SYSCONFDIR)/$(NAME)
LIBDIR		=	$(PREFIX)/lib
DATADIR		=	$(PREFIX)/var
WWWDIR		=	$(DATADIR)/www
LOGDIR		=	$(DATADIR)/log

INSTALLDIRS	=	$(BINDIR) $(CONFDIR) $(LIBDIR) $(DATADIR) $(LOGDIR)

WEBSERVLNK	=	$(WWWDIR)/$(NAME)

VPATH		:=	$(addprefix $(SRCDIR)/,$(SUBDIR)) $(SRCDIR)

# ==================================> Files <================================= #

CONFIG		=	Config.cpp \
				ConfigParser.cpp \
				Lexer.cpp \
				Location.cpp \
				Parser.cpp \
				ServerConfig.cpp
CORE		=	AcceptSocket.cpp \
				CgiHandler.cpp \
				Client.cpp \
				ConnectSocket.cpp \
				ListenSocket.cpp \
				Request.cpp \
				RequestParser.cpp \
				Response.cpp \
				Server.cpp \
				Socket.cpp \
				Webserv.cpp
UTILS		=	checkUri.cpp \
				file_utils.cpp \
				ft_inet_ntoa.cpp \
				ft_sleep.cpp \
				log.cpp \
				sockaddr_in.cpp \
				string_utils.cpp \
				trim.cpp

SRC			=	main.cpp $(CORE) $(CONFIG) $(UTILS)
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)

BIN			=	$(NAME)_$(BUILD)
TESTBIN		=	$(NAME)_test
LIB			:=	lib$(NAME).a

# ===============================> Environment <============================== #

CXX			=	c++
AR			:=	$(shell which ar)
RM			:=	$(shell which rm)
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

# ===============================> Compilation <============================== #

CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
CXXFLAGS	=	-Wall -Wextra -Werror
DEPFLAGS	=	-MT '$@ $(DEPDIR)/$(*F).d' -MMD -MP -MF $(DEPDIR)/$(*F).d

ifeq (test,$(strip $(MAKECMDGOALS)))
  CXXFLAGS	+=	-std=c++11
  CPPFLAGS	+=	-DLOG_OSTREAM=Log::OutputStream::kNone
else
  CXXFLAGS	+=	-std=c++98
  CPPFLAGS	+=	-DLOG_OSTREAM=Log::OutputStream::kBoth
endif

ifeq (debug,$(BUILD))
  CXXFLAGS	+=	-fsanitize=address,undefined -Og -g3 #-fno-omit-frame-pointer
# 				-fstack-protector-all \
# 				-Wpedantic \
# 				-Wshadow \
# 				-Wnon-virtual-dtor \
# 				-Wold-style-cast \
# 				-Wcast-align \
# 				-Wunused \
# 				-Wconversion \
# 				-Wsign-conversion \
# 				-Wnull-dereference \
# 				-Wdouble-promotion \
# 				-Wformat=2 \
# 				-Wmisleading-indentation
# ifeq (Linux,$(UNAME))
#  CXXFLAGS	+=	-Wduplicated-cond \
# 				-Wduplicated-branches \
# 				-Wlogical-op \
# 				-Wuseless-cast
# endif
  CPPFLAGS	+=	-DLOG_FILE=/tmp/webserv.log \
				-DLOG_LEVEL=Log::Level::kDebug \
				-DCONF_FILE=$(WORKDIR)/default.conf
else
  CPPFLAGS	+=	-DLOG_FILE=$(LOGDIR)/webserv.log \
				-DLOG_LEVEL=Log::Level::kError \
				-DCONF_FILE=$(SYSCONFDIR)/$(NAME)/default.conf
  CXXFLAGS	+=	-O3
endif

# =================================> Export <================================= #

export NAME
export DEPDIR
export BUILDIR
export INCLDIR
export LIBDIR
export CPPFLAGS
export CXXFLAGS
export WORKDIR
export LIB



################################################################################
#                                    RULES                                     #
################################################################################

all:			header $(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
	$(eval RULE = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
	@$(call run,$(RULE),$(COMPIL_MSG),$(B_CYAN))
	$(eval F=0)

$(DEPDIR):
	$(eval RULE = mkdir -p $@)
	@$(call run,$(RULE),$(MKDIR_MSG),$(B_BLUE))

ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),fclean)
    -include $(wildcard $(DEP))
  endif
endif

$(BIN):			$(OBJ) | $(INSTALLDIRS) $(WEBSERVLNK)
	$(eval RULE = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^)
	@$(call run,$(RULE),$(LINK_MSG),$(B_CYAN))
	$(eval F=1)

$(INSTALLDIRS):
	$(eval RULE = mkdir -p $@)
	@$(call run,$(RULE),$(MKDIR_MSG),$(B_BLUE))

$(WEBSERVLNK):
	$(eval RULE = ln -s $(WORKDIR)/www $(WEBSERVLNK))
	@$(call run,$(RULE),$(LN_MSG),$(B_BLUE))

$(LIB):			$(filter-out $(BUILDIR)/main.o, $(OBJ)) | header
	$(eval RULE = $(AR) rcs $@ $?)
	@$(call run,$(RULE),$(AR_MSG),$(B_BLUE))

install:		header $(BIN) $(LIB)
	$(eval RULE = install -m 755 $(BIN) $(BINDIR)/$(NAME) ;\
		cp default.conf $(CONFDIR)/)
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

uninstall:		header
	$(eval RULE = $(RM) -rf $(BINDIR)/$(NAME) $(WEBSERVLNK) $(CONFDIR))
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

test:			header $(LIB)
	$(MAKE) -C $(TESTDIR)
	$(eval RULE = chmod +x $(TESTBIN) ; ./$(TESTBIN))
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

clean:			header
	$(eval RULE = $(RM) -rf build *.log)
	@$(call run,$(RULE),clean)

fclean:			clean
	$(eval RULE = $(RM) -rf lib* webserv*)
	@$(call run,$(RULE),fclean)

re:				fclean all

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

.SUFFIXES:
.SUFFIXES:		.cpp .hpp .o .d

.PHONY:			all clean fclean install installdirs re test uninstall



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
AR_MSG		=	Creating/Updating
PROCESS_MSG	=	Processing
COMPIL_MSG	=	Compiling & assembling
FLAGS_MSG	=	Compiler flags
FLAGS		=	$(CXXFLAGS) $(CPPFLAGS)
LINK_MSG	=	Linking

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
	RUN_CMD = script -q "$(@F).log" $1 > /dev/null; \
				RESULT=$$? ; \
				sed -i "" -e "s/\^D//g" -e "s/[^[:print:]]//g" \
				-e "/^$$/d" "$(@F).log"
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
elif [ $(N) -eq 0 ]; then \
	printf "%-30b%-40b" "$(BLUE)$(2)" "$(3)$(@)$(NO_COLOR)"; \
fi
if ( echo "$(2)" | grep -q 'clean' ); then \
	$(1); \
	RESULT=$$?; \
else \
	$(RUN_CMD); \
fi ; \
if [ $(S) -ne 0 ]; then \
	:; \
elif [ $(N) -eq 0 -a $$RESULT -ne 0 ]; then \
	printf "%b\n" "$(RED)[KO]"; \
elif [ $(N) -eq 0 -a -s "$(@F).log" ]; then \
	printf "%b\n" "$(YELLOW)[WARN]"; \
elif [ $(N) -eq 0 ]; then  \
	printf "%b\n" "$(GREEN)[OK]"; \
fi; \
printf "%b" "$(NO_COLOR)"; \
cat "$(@F).log" 2> /dev/null; \
rm -f "$(@F).log"; \
exit $$RESULT
endef
