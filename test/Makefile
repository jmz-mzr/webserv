
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
CPPFLAGS	+=	$(addprefix -I, $(INCLDIR))
DEPFLAGS	=	-MT $@ -MMD -MP -MF $(DEPDIR)/$(*F).d
LDDFLAGS	=	$(LIB) $(GTEST) -pthread

#>	ENVIRONMENT
CXX			=	c++
AR			:=	$(shell which ar)
RM			:=	$(shell which rm)
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

$(BIN):			.info $(OBJ)
				@touch .info
				@$(call print_title,LINKING)
				@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDDFLAGS)
				@printf "%4s%-30s$(GRN)$(CHECK)$(RESET)\n" "" "$(@F)"
				@printf "\n\n"

.info:			$(wildcard $(BUILDIR)/*.o)
				@$(call print_title,INFO)
				@printf "$(UWHT)%s$(RESET)\t$(FBLU)%s$(RESET)\n\n" "Build profile:" "$(BUILD)"
				@printf "$(UWHT)%s$(RESET)\n" "Preproc opts:"
				@$(call print_str_format_width,$(sort $(CPPFLAGS)),30)
				@printf "$(UWHT)%s$(RESET)\n" "Compiler opts:"
				@$(call print_str_format_width,$(sort $(CXXFLAGS)),30)
				@$(call print_title,COMPILE)
				@touch $@