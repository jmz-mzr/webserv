################################################################################
#                                    CONFIG                                    #
################################################################################

# =================================> Project <================================ #

NAME		=	webserv
AUTHOR		=	jmazoyer flohrel mtogbe

BUILD		?=	debug
PREFIX		?=	$(shell [ -d "${HOME}/goinfre" ] && [ -w "${HOME}/goinfre" ] \
				&& echo "${HOME}/goinfre" || echo "/usr/local")

ifeq ($(PREFIX),)
  $(error 'Installation directory undefined')
endif

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

BINDIR		=	$(PREFIX)/bin
SYSCONFDIR	=	$(PREFIX)/etc
CONFDIR		=	$(SYSCONFDIR)/$(NAME)
LIBDIR		=	$(PREFIX)/lib
DATADIR		=	$(PREFIX)/var
WWWDIR		=	$(DATADIR)/www
LOGDIR		=	$(DATADIR)/log

INSTALLDIRS	=	$(BINDIR) $(CONFDIR) $(LIBDIR) $(DATADIR) $(LOGDIR) $(WWWDIR)

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

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98
CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
DEPFLAGS	=	-MT '$@ $(DEPDIR)/$(*F).d' -MMD -MP -MF $(DEPDIR)/$(*F).d

ifeq (test,$(strip $(MAKECMDGOALS)))
  CPPFLAGS	+=	-DLOG_OSTREAM=Log::OutputStream::kNone
else
  CPPFLAGS	+=	-DLOG_OSTREAM=Log::OutputStream::kBoth
endif

ifeq (debug,$(BUILD))
  CXXFLAGS	+=	-fsanitize=address,undefined -Og -g3# -fno-omit-frame-pointer
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
				-DCONF_FILE=$(WORKDIR)/default.conf \
				-DWEBSERV_ROOT=$(WORKDIR)/www
else
  CXXFLAGS	+=	-O3
  CPPFLAGS	+=	-DLOG_FILE=$(LOGDIR)/webserv.log \
				-DLOG_LEVEL=Log::Level::kError \
				-DCONF_FILE=$(SYSCONFDIR)/$(NAME)/default.conf \
				-DWEBSERV_ROOT=$(DATADIR)/www/webserv
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

$(BIN):			$(OBJ)
	$(eval RULE = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^)
	@$(call run,$(RULE),$(LINK_MSG),$(B_CYAN))
	$(eval F=1)

$(INSTALLDIRS):
	$(eval RULE = mkdir -p $@)
	@$(call run,$(RULE),$(MKDIR_MSG),$(B_BLUE))

$(WEBSERVLNK):
	$(eval RULE = ln -s $(WORKDIR)/www $(WEBSERVLNK))
	@$(call run,$(RULE),$(LN_MSG),$(B_BLUE))
	$(eval RULE = chmod +x $(WORKDIR)/www/webserv.42.fr/{index.php,cgi-bin/info.php})
	@$(call run,$(RULE),$(CHMOD_MSG),$(B_BLUE))

$(LIB):			$(filter-out $(BUILDIR)/main.o, $(OBJ)) | header
	$(eval RULE = $(AR) rcs $@ $?)
	@$(call run,$(RULE),$(AR_MSG),$(B_BLUE))

install:		header $(BIN) $(LIB) | $(INSTALLDIRS) $(WEBSERVLNK)
	$(eval RULE = install -m 755 $(BIN) $(BINDIR)/$(NAME) ;\
		cp default.conf $(CONFDIR)/)
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

uninstall:		header
	$(eval RULE = $(RM) -rf $(BINDIR)/$(NAME) $(WEBSERVLNK) $(CONFDIR))
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

test:			header $(LIB)
	$(eval RULE = $(MAKE) -C $(TESTDIR))
	@$(call run,$(RULE),$(MAKE_MSG),$(TESTDIR))
	@chmod +x $(TESTBIN)
	$(eval RULE = ./$(TESTBIN) > /dev/null)
	@$(call run,$(RULE),$(PROCESS_MSG),$(B_BLUE))

clean:			header
	$(eval RULE = $(RM) -rf build *.log)
	@$(call run,$(RULE),clean)

fclean:			clean
	$(eval RULE = $(RM) -rf lib* webserv*)
	@$(call run,$(RULE),fclean)

re:				fclean all

# ================================> Functions <=============================== #

include $(WORKDIR)/.functions.mk

# =============================> Special Targets <============================ #

.SUFFIXES:
.SUFFIXES:		.cpp .hpp .o .d

.PHONY:			all install uninstall test clean fclean re
