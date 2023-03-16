################################################################################
#                                    CONFIG                                    #
################################################################################

# =================================> Project <================================ #

override BUILD = debug

# ===============================> Directories <============================== #

INCLDIR		:=	$(addprefix $(WORKDIR)/,$(INCLDIR)) \
				/usr/local/include /usr/local/include/gtest
BUILDIR		:=	$(addprefix $(WORKDIR)/,$(BUILDIR))
DEPDIR		:=	$(addprefix $(WORKDIR)/,$(DEPDIR))

VPATH		=	src

# ==================================> Files <================================= #

SRC			=	ConfigParser_test.cpp \
				trim_test.cpp
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)

BIN			=	$(WORKDIR)/$(NAME)_test
LIB			:=	$(addprefix $(WORKDIR)/,$(LIB))
GTEST		:=	$(addprefix $(LIBDIR)/, libgtest_main.a libgtest.a)

# ===============================> Environment <============================== #

CXX			=	c++
AR			:=	$(shell which ar)
RM			:=	$(shell which rm)
SHELL		:=	$(shell which bash)
UNAME		:=	$(shell uname -s)

# ===============================> Compilation <============================== #

CXXFLAGS	:= $(CXXFLAGS:c++98=c++14)
CPPFLAGS	+=	$(addprefix -I, $(INCLDIR))
DEPFLAGS	=	-MT '$@ $(DEPDIR)/$(*F).d' -MMD -MP -MF $(DEPDIR)/$(*F).d
LDDFLAGS	=	$(LIB) $(GTEST) -pthread



################################################################################
#                                    RULES                                     #
################################################################################

all:			$(BIN)

$(BUILDIR)/%.o:	%.cpp | $(DEPDIR)
	$(eval RULE = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
	@$(call run,$(RULE),$(COMPIL_MSG),$(B_CYAN))
	$(eval F=0)

$(DEPDIR):
	$(eval RULE = mkdir -p $@)
	@$(call run,$(RULE),$(MKDIR_MSG),$(B_BLUE))

-include $(wildcard $(DEP))

$(BIN):			$(OBJ)
	$(eval RULE = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDDFLAGS))
	@$(call run,$(RULE),$(LINK_MSG),$(B_CYAN))
	$(eval F=1)

# ================================> Functions <=============================== #

include $(WORKDIR)/.functions.mk

# =============================> Special Targets <============================ #

.PHONY:			all
