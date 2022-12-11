include utils.mk

#################
##> Variables <##
#################

NAME		=	webserv

#>	DIRECTORIES
SUBDIR		=	config core utils
TESTDIR		=	test/src
INCLDIR		:=	$(addprefix include/,$(LIBDIR)) include
BUILDIR		=	build
DEPDIR		=	$(BUILDIR)/.deps

#>	FILES
VPATH		=	$(addprefix src/,$(SUBDIR)) src
SRC			=	main.cpp
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
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
TEST		:=	$(foreach file, $(wildcard $(TESTDIR)/*.cpp), $(notdir $(file)))

#>	COMPILATION FLAGS
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98
CXXFLAGS	+=	-DWEBSERV_ROOT=\"$(shell pwd)\"
CPPFLAGS	:=	$(addprefix -I, $(INCLDIR))
LDFLAGS		:=	$(addprefix -L, $(LIBDIR)) $(addprefix -l, $(LIB))
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$(*F).d

ifeq (test,$(strip $(MAKECMDGOALS)))						# if testing
	CXXFLAGS	+=	-fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g3 -DCONF_LOG_OUT="Logger::kNone"
	BIN			=	$(NAME).test
	SRC			+=	$(TEST:%.test.cpp=%.cpp) $(TEST)
	VPATH		+=	test/src
else
ifeq (debug,$(strip $(MAKECMDGOALS)))						# if debugging
	CXXFLAGS	+=	-fsanitize=address,undefined -g3
endif
	CXXFLAGS 	+=	-DDOCTEST_CONFIG_DISABLE
	BIN			=	$(NAME)
	SRC			+=	$(CORE) $(CONFIG) $(UTILS)
endif

#>	ENVIRONMENT
CXX			=	c++
RM			=	/bin/rm -rf
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

#############
##> Rules <##
#############

.PHONY:			all clean fclean header

all:			header $(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $(BUILDIR)/$(@F)
				@printf "%9s► %-45s$(GRN)$(CHECK)$(RESET)\n" "" "$<"

$(DEPDIR):
				@$(call print_title,COMPILE)
				@mkdir -p $@

-include $(wildcard $(DEP))

$(BIN):			$(OBJ)
				@$(call print_title,LINK)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
				@printf "%9s► %-45s$(GRN)$(CHECK)$(RESET)\n" "" "./$@"
				@printf "\n\n"

clean:			header
				@$(call print_title,DELETE)
				@printf "%9s► "
				$(RM) $(BUILDIR)
ifeq (clean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

fclean:			clean
				@printf "%9s► "
				$(RM) $(NAME) $(NAME).test
ifeq (fclean,$(MAKECMDGOALS))
	@printf "\n\n"
endif

re:				fclean $(BIN)

debug:			all

test:			all
				@chmod +x $(BIN)
				@./$(BIN)

header:
				@cat misc/header.txt