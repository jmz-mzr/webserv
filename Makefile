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
TESTDIR		=	tests/cpp_gtest
WORKDIR		:=	$(realpath .)

ifeq ($(PREFIX),)
  PREFIX	=	/usr/local
endif

EXEC_PREFIX	=	$(PREFIX)
BINDIR		=	$(PREFIX)/bin
SYSCONFDIR	=	$(PREFIX)/etc
LIBDIR		=	$(PREFIX)/lib
DATADIR		=	$(PREFIX)/var
WWWDIR		=	$(DATADIR)/www
LOGDIR		=	$(DATADIR)/log

#>	FILES
VPATH		:=	$(addprefix $(SRCDIR)/,$(SUBDIR)) $(SRCDIR)
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

ifeq (test,$(strip $(MAKECMDGOALS)))
  CXXFLAGS +=	-std=c++11
  CPPFLAGS +=	-DLOG_OSTREAM=Log::OutputStream::kNone
else
  CXXFLAGS +=	-std=c++98
  CPPFLAGS +=	-DLOG_OSTREAM=Log::OutputStream::kBoth
endif

ifneq ($(filter install, $(strip $(MAKECMDGOALS))),)
  override BUILD = release
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
# endif
  CPPFLAGS +=	-DLOG_FILE=/tmp/webserv.log \
				-DLOG_LEVEL=Log::Level::kDebug \
				-DCONF_FILE=$(WORKDIR)/default.conf
else
  CPPFLAGS +=	-DLOG_FILE=$(LOGDIR)/webserv.log \
				-DLOG_LEVEL=Log::Level::kError \
				-DCONF_FILE=$(SYSCONFDIR)/$(NAME)/default.conf
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

.PHONY:			all clean fclean install installdirs re test uninstall

all:			$(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(DEPDIR):
				mkdir -p $@

-include $(wildcard $(DEP))

$(BIN):			$(OBJ)
				$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

$(LIB):			$(filter-out $(BUILDIR)/main.o, $(OBJ))
				$(AR) rcs $@ $?

installdirs:
				install -d $(BINDIR) $(DATADIR) $(SYSCONFDIR)/$(NAME) \
					$(WWWDIR) $(LOGDIR)

install:		$(BIN) $(LIB) installdirs
				install -m 755 $(BIN) $(BINDIR)/$(NAME)
				rm -rf $(WWWDIR)/webserv
				ln -si $(WORKDIR)/www $(WWWDIR)/webserv
				cp default.conf $(SYSCONFDIR)/$(NAME)/

uninstall:
				$(RM) $(BINDIR)/$(NAME)
				$(RM) -rf $(WWWDIR)/webserv
				$(RM) -rf $(SYSCONFDIR)/$(NAME)

test:			$(LIB)
				$(MAKE) -C $(TESTDIR)
				chmod +x $(TESTBIN)
				./$(TESTBIN)

clean:
				$(RM) -rf build

fclean:			clean
				$(RM) -rf lib* webserv*

re:				fclean all
