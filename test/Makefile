
#################
##> Variables <##
#################

#>	DIRECTORIES
INCLDIR		:=	$(addprefix $(WORKDIR)/,$(INCLDIR)) /usr/local/include/gtest
BUILDIR		:=	$(addprefix $(WORKDIR)/,$(BUILDIR))
DEPDIR		:=	$(addprefix $(WORKDIR)/,$(DEPDIR))
VPATH		=	src

#>	FILES
BIN			=	$(WORKDIR)/$(NAME)_test
SRC			=	trim_test.cpp \
				Logger_test.cpp
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
LIB			:=	$(addprefix $(WORKDIR)/,$(LIB))
GTEST		:=	$(addprefix $(LIBDIR)/, libgtest_main.a libgtest.a)

include $(WORKDIR)/utils.mk

override BUILD = debug

#>	COMPILATION FLAGS
CPPFLAGS	=	$(addprefix -I, $(INCLDIR))
CXXFLAGS	+=	-std=c++11 -fsanitize=address,undefined -fno-omit-frame-pointer
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$(*F).d
LDDFLAGS	=	$(LIB) $(GTEST) -pthread

#>	ENVIRONMENT
CXX			=	c++
AR			=	/bin/ar rcs
RM			=	/bin/rm -rf
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

#############
##> Rules <##
#############

.PHONY:			all clean fclean

all:			$(BIN)

$(BUILDIR)/%.o:	%.cpp
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "test/$<"

-include $(wildcard $(DEP))

$(BIN):			.title $(OBJ)
				@touch .title
				@$(call print_title,LINKING)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDDFLAGS)
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$(@F)"
				@printf "\n\n"

.title:			$(wildcard $(BUILDIR)/*.o)
				@$(call print_title,COMPILING)
				@touch $@