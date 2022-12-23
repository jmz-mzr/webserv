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
  PREFIX := /usr/local
endif
EXEC_PREFIX	=	$(PREFIX)
BINDIR		=	$(PREFIX)/bin
SYSCONFDIR	=	$(PREFIX)/etc
LIBDIR		=	$(PREFIX)/lib
DATAROOTDIR	=	$(PREFIX)/share
DATADIR		=	$(DATAROOTDIR)

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
SRC			=	$(CORE) $(CONFIG) $(UTILS)
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
BIN			:=	$(NAME)_$(BUILD)
TESTBIN		=	$(NAME)_test
LIB			:=	lib$(NAME).a

#>	COMPILATION FLAGS
CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
CXXFLAGS	=	-Wall -Wextra -Werror
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$(*F).d

#>	ENVIRONMENT
CXX			=	c++
AR			=	/bin/ar rcs
RM			=	/bin/rm -rf
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

ifneq ($(filter-out debug release,$(BUILD)),)
  $(error '$(BUILD)' is not a correct value. Build options are 'debug' or 'release')
endif

ifeq (install,$(strip $(MAKECMDGOALS)))
  override BUILD = release
endif

ifneq (test,$(strip $(MAKECMDGOALS)))
  SRC += main.cpp
  CXXFLAGS += -std=c++98
else
  CPPFLAGS += -DLOG_OSTREAM="webserv::Logger::kNone"
endif

ifeq (debug,$(BUILD))
  CXXFLAGS +=	-fsanitize=address,undefined -Og \
				-fstack-protector-all \
				-Wpedantic \
				-Wshadow \
				-Wnon-virtual-dtor \
				-Wold-style-cast \
				-Wcast-align \
				-Wunused \
				-Wconversion \
				-Wsign-conversion \
				-Wnull-dereference \
				-Wdouble-promotion \
				-Wformat=2 \
				-Wmisleading-indentation \
				-Wduplicated-cond \
				-Wduplicated-branches \
				-Wlogical-op \
				-Wuseless-cast
else
  CPPFLAGS += -DNDEBUG
  CXXFLAGS += -O3 -march=native
endif

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

.INTERMEDIATE:

.SUFFIXES:
.SUFFIXES:		.cpp .hpp .o .d

.PHONY:			all check clean fclean test

all:			$(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$(subst src/,,$<)"

$(DEPDIR):
				@mkdir -p $@

-include $(wildcard $(DEP))

.title:			$(wildcard $(BUILDIR)/*.o)
				@$(call print_title,COMPILE)
				@touch $@

.header:
				@head -n8 README.md
				@touch $@

.info:
				@$(call print_title,INFO)
				@printf "$(UWHT)%s$(RESET)\t$(FBLU)%s$(RESET)\n\n" "Build profile:" "$(BUILD)"
				@printf "$(UWHT)%s$(RESET)\n" "Preproc opts:"
				@$(call print_str_format_width,$(sort $(CPPFLAGS)),30)
				@printf "$(UWHT)%s$(RESET)\n" "Compiler opts:"
				@$(call print_str_format_width,$(sort $(CXXFLAGS)),30)
				@touch $@

$(BIN):			.header .info .title $(OBJ)
				@touch .title
				@$(call print_title,LINK)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$@"
				@printf "\n\n"

$(LIB):			.header .info .title $(OBJ)
				@touch .title
				@$(call print_title,ARCHIVE)
				@$(AR) $@ $?
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$@"

#> WORK IN PROGRESS
install:		$(BIN) $(LIB)
				@$(call print_title,INSTALL)
				@sudo install -d $(DESTDIR)$(BINDIR)
				@sudo install -m 644 $(BIN) $(DESTDIR)$(BINDIR)
				@printf "%4s%s  🡆  %s\n" "./$(BIN)" "" "$(BINDIR)/$(BIN)"
				@sudo cp -R conf/default.conf $(SYSCONFDIR)/webserv/
				@printf "%4s%s  🡆  %s\n" "./$(BIN)" "" "$(BINDIR)/$(BIN)"
				@printf "\n\n"

test:			$(LIB)
				@$(MAKE) -sC $(TESTDIR)
				@chmod +x $(TESTBIN)
				@$(call print_title,TEST)
				@./$(TESTBIN)

clean:			.header
				@$(call print_title,DELETE)
				@printf "%4s" ""
				$(RM) build .title .info .header
ifeq (clean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

fclean:			clean
				@printf "%4s" ""
				$(RM) lib* webserv*
ifeq (fclean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

re:				fclean all