#################
##> Variables <##
#################

NAME		=	webserv

#>	DIRECTORIES
DIRS		=	config core utils
VPATH		=	src $(addprefix src/,$(DIRS))
LIBDIR		=
LOGFILE		=	$(NAME).log
INCLDIR		=	include $(addprefix include/,$(LIBDIR))
BUILDIR		=	build
DEPDIR		=	$(BUILDIR)/.deps

#>	FILES
SRC			=	main.cpp \
				Config.cpp \
				ServerConfig.cpp \
				Socket.cpp \
				ListenSocket.cpp \
				AcceptSocket.cpp \
				ConnectSocket.cpp \
				Webserv.cpp \
				Server.cpp \
				Client.cpp \
				Request.cpp \
				Response.cpp \
				Logger.cpp \
				ft_memset.cpp \
				ft_inet_ntoa.cpp \
				ft_sleep.cpp
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)

#>	COMPILATION FLAGS
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -fsanitize=address,undefined -g3
CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
LDFLAGS		=	$(addprefix -L, $(LIBDIR)) $(addprefix -l, $(LIB))
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

ifeq (,$(strip $(findstring test, $(MAKECMDGOALS))))		# if not testing
	CXXFLAGS 	+=	-DDOCTEST_CONFIG_DISABLE
	BIN			=	$(NAME)
else
	CXXFLAGS	+=	-fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g3 -DCONF_LOG_OUT=kNone
	BIN			=	test_runner
endif

#>	ENVIRONMENT
CXX			=	c++
RM			=	/bin/rm -rf
SHELL		:=	$(shell which zsh)
UNAME		:=	$(shell uname -s)

#>	FG COLORS
DEFAULT		=	\033[0m
BLACK		=	\033[1;30m
RED			=	\033[1;31m
GREEN		=	\033[1;32m
YELLOW		=	\033[1;33m
BLUE		=	\033[1;34m
MAGENTA 	=	\033[1;35m
CYAN 		=	\033[1;36m
WHITE 		=	\033[1;107m

#>	TERMCAPS
UP			=	\033[1A
DELETE		=	\033[2K
DELPREV		=	$(DELETE)\r$(UP)

#>	EMOJIS
CHECK		=	\xE2\x9C\x94
CROSS		=	\xE2\x9D\x8C
TRASH		=	\xF0\x9F\x97\x91

#>	BOX DRAWING
DHORIZ		=	\xE2\x95\x90
DVERT		=	\xE2\x95\x91
TOP-LEFT	=	\xE2\x95\x94
TOP-RIGHT	=	\xE2\x95\x97
BOT-LEFT	=	\xE2\x95\x9A
BOT-RIGHT	=	\xE2\x95\x9D
PAD_CHAR	=	\xE2\x96\x91
VERT		=	\xE2\x94\x83
HORIZ		=	\xE2\x94\x81
BOT-LEFT2	=	\xE2\x94\x97
BOT-RIGHT2	=	\xE2\x94\x9B

#>	DISPLAY
WIDTH		=	48
NAME_SIZE	=	$(shell NAME='$(NAME)'; printf "$${\#NAME}")
PAD_WIDTH	=	$(shell printf "$$(((($(WIDTH) - $(NAME_SIZE)) / 2) - 1))")
PAD_PREC	=	$(shell printf "$$(($(PAD_WIDTH) / 1.5))")
PAD_STR		=	$(shell printf '$(PAD_CHAR)%.0s' {1..$$(($(WIDTH) - 2))})
HORIZ_PAD	=	$(shell printf '$(DHORIZ)%.0s' {1..$$(($(WIDTH) - 2))})
LEFT_PAD	=	$(shell printf '$(DVERT)%-*.*s' $(PAD_WIDTH) $(PAD_PREC) $(PAD_STR))
RIGHT_PAD	=	$(shell printf '%*.*s$(DVERT)' $$(($(PAD_WIDTH) + $(NAME_SIZE) % 2)) $(PAD_PREC) $(PAD_STR))
BODY_WIDTH	=	$(shell printf "$$(($(WIDTH) - 5))")
HORIZ_LINE	=	$(shell printf '$(HORIZ)%.0s' {1..$$(($(WIDTH) - 2))})
FOOTER		=	$(shell printf '$(BOT-LEFT2)$(HORIZ_LINE)$(BOT-RIGHT2)')


#############
##> Rules <##
#############

.PHONY:			all clean fclean header re test

all:			header $(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@printf "\n$(YELLOW)Compiling $@ and generating/checking make dependency file...$(DEFAULT)"
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf '$(DELPREV)$(VERT) %-*s$(GREEN)$(CHECK)$(DEFAULT) $(VERT)\n' $(BODY_WIDTH) "$(notdir $<) -> $(notdir $@)"

$(BIN):			$(OBJ)
				@printf "\n$(YELLOW)Linking source files and generating $@ binary...$(DEFAULT)"
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
				@printf "$(DELPREV)$(VERT) $(GREEN)%-*s$(CHECK)$(DEFAULT) $(VERT)\n" $(BODY_WIDTH) "./$(BIN)"
				@printf "$(FOOTER)\n"

$(DEPDIR):
				@printf "\n$(YELLOW)Creating $@ folder...$(DEFAULT) $(VERT)"
				@mkdir -p $@
				@printf "$(DELPREV)"
$(DEP):
-include $(wildcard $(DEP))

clean:			header
				@printf "\n$(YELLOW)Deleting object and dependency files...$(DEFAULT)"
				@$(RM) $(OBJ)
				@printf "$(DELPREV)$(VERT) %-*s$(DEFAULT)$(TRASH) $(VERT)\n" $(BODY_WIDTH) "Build files"
ifeq (clean,$(MAKECMDGOALS))
	@printf "$(FOOTER)\n"
endif

fclean:			clean
				@printf "\n$(YELLOW)Deleting build directory...$(DEFAULT)"
				@$(RM) $(BUILDIR) $(LOGFILE) webserv test_runner
				@printf "$(DELPREV)$(VERT) %-*s$(DEFAULT)$(TRASH) $(VERT)\n" $(BODY_WIDTH) "Build directory"
				@printf "$(VERT) %-*s$(DEFAULT)$(TRASH) $(VERT)\n" $(BODY_WIDTH) "ELF files"
ifeq (fclean,$(MAKECMDGOALS))
	@printf "$(FOOTER)\n"
endif

header:
				@printf "$(TOP-LEFT)$(HORIZ_PAD)$(TOP-RIGHT)\n"
				@printf "$(LEFT_PAD)$(BLUE)$(NAME)$(DEFAULT)$(RIGHT_PAD)\n"
				@printf "$(BOT-LEFT)$(HORIZ_PAD)$(BOT-RIGHT)\n"

re:				fclean $(BIN)

test:			fclean all
				@chmod +x $(BIN)
				@./$(BIN) 4444
