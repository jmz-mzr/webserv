
#################
##> Variables <##
#################

#>	DIRECTORIES
INCLDIR		:=	$(addprefix $(ROOTDIR)/,$(INCLDIR)) /usr/local/include/gtest
BUILDIR		:=	$(addprefix $(ROOTDIR)/,$(BUILDIR))
DEPDIR		:=	$(addprefix $(ROOTDIR)/,$(DEPDIR))
VPATH		=	src

#>	FILES
BIN			=	$(NAME).test
GTEST		=	/usr/local/lib/libgtest_main.a /usr/local/lib/libgtest.a
SRC			=	trim_test.cpp
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
LIB			:=	$(addprefix $(ROOTDIR)/,$(LIB))

include $(ROOTDIR)/utils.mk

#>	COMPILATION FLAGS
CPPFLAGS	+=	$(addprefix -I, $(INCLDIR)) -DCONF_LOG_OUT="Logger::kNone" 
CXXFLAGS	+=	-fsanitize=address,undefined -fno-omit-frame-pointer
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

.PHONY:			all clean fclean header

all:			$(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
				@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
				@printf "%4s %-55s$(GRN)$(CHECK)$(RESET)\n" "" "./$<"

-include $(wildcard $(DEP))

$(BIN):			$(OBJ)
				@$(call print_title,LINKING)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDDFLAGS)
				@printf "%3s %-55s$(GRN)$(CHECK)$(RESET)\n" "" "./$(BIN)"
				@printf "\n\n"
				@chmod +x $(BIN)
				@$(call print_title,TESTING)
				@./$(BIN)
