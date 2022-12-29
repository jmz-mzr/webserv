include utils.mk

#################
##> Variables <##
#################

NAME		=	webserv
BUILD		?=	debug

#>	DIRECTORIES
SRCDIR		=	src
SUBDIR		=	config core utils
INCLDIR		=	include
BUILDIR		=	build/$(BUILD)
DEPDIR		=	$(BUILDIR)/.deps
TESTDIR		=	test
WORKDIR		:=	$(realpath .)

ifeq ($(PREFIX),)
  PREFIX	= /usr/local
endif

EXEC_PREFIX	=	$(PREFIX)
BINDIR		=	$(PREFIX)/bin
SYSCONFDIR	=	$(PREFIX)/etc
LIBDIR		=	$(PREFIX)/lib
DATAROOTDIR	=	$(PREFIX)/share
DATADIR		=	$(DATAROOTDIR)
LOGDIR		=	/var/log

#>	FILES
VPATH		:=	$(addprefix $(SRCDIR)/,$(SUBDIR)) $(SRCDIR)
CONFIG		=	Config.cpp \
				ConfigParser.cpp \
				Lexer.cpp \
				Location.cpp \
				Parser.cpp \
				ServerConfig.cpp
CORE		=	AcceptSocket.cpp \
				Client.cpp \
				ConnectSocket.cpp \
				ListenSocket.cpp \
				Request.cpp \
				RequestParser.cpp \
				Response.cpp \
				Server.cpp \
				Socket.cpp \
				Webserv.cpp
UTILS		=	ft_charcmp_icase.cpp \
				ft_inet_ntoa.cpp \
				ft_sleep.cpp \
				ft_str_tolower.cpp \
				ft_strcmp_icase.cpp \
				sockaddr_in.cpp \
				Logger.cpp \
				trim.cpp
SRC			=	main.cpp $(CORE) $(CONFIG) $(UTILS)
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
BIN			=	$(NAME)_$(BUILD)
TESTBIN		=	$(NAME)_test
LIB			:=	lib$(NAME).a

#>	COMPILATION FLAGS
CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
CXXFLAGS	=	-Wall -Wextra -Werror
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$(*F).d

#>	ENVIRONMENT
CXX			=	c++
AR			:=	$(shell which ar)
RM			:=	$(shell which rm)
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

ifneq ($(filter-out debug release,$(BUILD)),)
  $(error '$(BUILD)' is not a correct value. Build options are 'debug' or 'release')
endif

ifeq (install,$(strip $(MAKECMDGOALS)))
  override BUILD = release
endif

ifeq (test,$(strip $(MAKECMDGOALS)))
  CXXFLAGS +=	-std=c++11
  CPPFLAGS +=	-DLOG_OSTREAM=webserv::Logger::kNone
else
  CXXFLAGS +=	-std=c++98
  CPPFLAGS +=	-DLOG_OSTREAM=webserv::Logger::kBoth
endif

ifeq (debug,$(BUILD))
  CXXFLAGS +=	-fsanitize=address,undefined -Og
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
#   CXXFLAGS +=	-Wduplicated-cond \
# 				-Wduplicated-branches \
# 				-Wlogical-op \
# 				-Wuseless-cast
endif
  CPPFLAGS +=	-DNDEBUG -DLOG_LEVEL=webserv::Logger::kDebug \
  				-DLOG_FILE=webserv.log
else
  CPPFLAGS +=	-DNDEBUG -DLOG_LEVEL=webserv::Logger::kError \
  				-DLOG_FILE=$(LOGDIR)/webserv.log \
				-DDEFAULT_CONF=$(SYSCONFDIR)/$(NAME)/conf/test.conf
  CXXFLAGS +=	-O3
endif

export NAME
export DEPDIR
export BUILDIR
export INCLDIR
export LIBDIR
export CPPFLAGS
export CXXFLAGS
export WORKDIR
export LIB

#############
##> Rules <##
#############

.SUFFIXES:
.SUFFIXES:		.cpp .hpp .o .d

.PHONY:			all clean fclean test

all:			.info $(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$(subst src/,,$<)"

$(DEPDIR):
				@mkdir -p $@

-include $(wildcard $(DEP))

.info:			$(wildcard $(BUILDIR)/*.o)
ifeq (0,$(MAKELEVEL))
				@cat .header
endif
				@$(call print_title,INFO)
				@printf "$(UWHT)%s$(RESET)\t$(FBLU)%s$(RESET)\n\n" "Build profile:" "$(BUILD)"
				@printf "$(UWHT)%s$(RESET)\n" "Preproc opts:"
				@$(call print_str_format_width,$(CPPFLAGS),30)
				@printf "$(UWHT)%s$(RESET)\n" "Compiler opts:"
				@$(call print_str_format_width,$(CXXFLAGS),30)
				@$(call print_title,COMPILE)
				@touch $@

$(BIN):			$(OBJ)
				@touch .info
				@$(call print_title,LINK)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$@"
				@printf "\n\n"

$(LIB):			$(filter-out $(BUILDIR)/main.o, $(OBJ))
				@$(call print_title,ARCHIVE)
				@$(AR) rcs $@ $?
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$@"

install:		.info $(BIN) $(LIB)
				@$(call print_title,INSTALL)
				@install -d $(BINDIR)
				@install -m 755 $(BIN) $(BINDIR)/$(NAME)
				@printf "%4s%s  🡆  %s\n" "$(BIN)" "" "$(BINDIR)/$(NAME)"
				@cp -R conf $(SYSCONFDIR)/$(NAME)/
				@printf "%4s%s  🡆  %s\n" "conf/" "" "$(SYSCONFDIR)/$(NAME)/conf"
				@printf "\n\n"

test:			.info $(LIB)
				@touch .info
				@$(MAKE) -sC $(TESTDIR)
				@chmod +x $(TESTBIN)
				@$(call print_title,TEST)
				@./$(TESTBIN)

clean:
				@cat .header
				@$(call print_title,DELETE)
				@printf "%4s" ""
				$(RM) -rf build .info
ifeq (clean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

fclean:			clean
				@printf "%4s" ""
				$(RM) -rf lib* webserv*
ifeq (fclean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

re:				fclean
				@$(MAKE) -s all