include utils.mk

#################
##> Variables <##
#################

NAME		=	webserv
BUILD		?=	debug

#>	DIRECTORIES
SUBDIR		=	config core utils
INCLDIR		=	include
LIBDIR		=	lib/$(BUILD)
BINDIR		=	bin/$(BUILD)
BUILDIR		=	build/$(BUILD)
DEPDIR		=	$(BUILDIR)/.deps
TESTDIR		=	test
ROOTDIR		:=	$(realpath .)
INSTALL_DIR	:=	$(HOME)/.local
VPATH		:=	$(addprefix src/,$(SUBDIR)) src

#>	FILES
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
BIN			=	$(BINDIR)/$(NAME)
LIB			:=	$(LIBDIR)/lib$(NAME).a
LOGFILE		=	$(NAME).log

#>	COMPILATION FLAGS
CPPFLAGS	=	$(addprefix -I, $(INCLDIR)) -DWEBSERV_ROOT=\"$(ROOTDIR)\"
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
ifeq (test,$(strip $(MAKECMDGOALS)))
  override BUILD = debug
  CXXFLAGS += -std=c++11
else
ifeq (install,$(strip $(MAKECMDGOALS)))
  override BUILD = release
endif
  SRC += main.cpp
  CXXFLAGS += -std=c++98
endif
ifeq ($(BUILD),debug)
	CXXFLAGS += -fsanitize=address,undefined -Og \
				-fstack-protector-all \
				-Wshadow \
				-Wnon-virtual-dtor \
				-Wold-style-cast \
				-Wcast-align \
				-Wunused \
				-Wpedantic \
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
export ROOTDIR
export BUILDIR
export INCLDIR
export CPPFLAGS
export CXXFLAGS
export LIB

#############
##> Rules <##
#############

.PHONY:			all clean fclean header

all:			header $(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf "%4s %-55s$(GRN)$(CHECK)$(RESET)\n" "" "./$<"

$(DEPDIR):
				@$(call print_title,COMPILATING)
				@mkdir -p $@

-include $(wildcard $(DEP))

$(BIN):			$(OBJ) | $(BINDIR)
				@$(call print_title,LINKING)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^
				@printf "%4s %-55s$(GRN)$(CHECK)$(RESET)\n" "" "./$@"
				@printf "\n\n"

$(BINDIR):
				@mkdir -p $@

clean:			header
				@$(call print_title,DELETING)
				@printf "%4s "
				$(RM) build
ifeq (clean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

fclean:			clean
				@printf "%4s "
				$(RM) lib bin $(LOGFILE)
ifeq (fclean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

$(LIB):			$(OBJ) | $(LIBDIR)
				@$(call print_title,ARCHIVING)
				@$(AR) $@ $^
				@printf "%4s %-55s$(GRN)$(CHECK)$(RESET)\n" "" "./$@"

$(LIBDIR):
				@mkdir -p $@

install:		header $(BIN)
				@$(call print_title,INSTALLING)
				@cp $(BINDIR)/$(BIN) $(INSTALL_DIR)
				@printf "%4s %s   ►   %s\n" "" "$(LIBDIR)/$(BIN)" "$(INSTALL_DIR)/$(BIN)"
				@printf "\n\n"

re:				fclean $(BIN)

test:			header $(LIB)
				@$(MAKE) -sC $(TESTDIR)

header:
				@cat misc/header.txt