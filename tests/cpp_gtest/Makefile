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
SRC			=	ConfigParser_test.cpp \
				trim_test.cpp \
				Logger_test.cpp
OBJ			=	$(SRC:%.cpp=$(BUILDIR)/%.o)
DEP			=	$(SRC:%.cpp=$(DEPDIR)/%.d)
LIB			:=	$(addprefix $(WORKDIR)/,$(LIB))
GTEST		:=	$(addprefix $(LIBDIR)/, libgtest_main.a libgtest.a)

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
				$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

-include $(wildcard $(DEP))

$(BIN):			$(OBJ)
				$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDDFLAGS)
