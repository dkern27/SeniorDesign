#DRS_Plots - Dense Ring Simulation Plots

#To build : 'make'
#To allow debugging : 'make debug'
#To remove compiled program (does not remove debug .dSYM): 'make clean'


CC=g++
CFLAGS = -Wall `root-config --cflags`
# -Wall, shows all warnings, -w suppresses all warnings, -v verbose output
ROOTBUILD = $(CC) $(CFLAGS)
#ROOTLIBS=-L/opt/local/lib/root5 -lGui -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lUnuran -lThread -pthread -lm -ldl -rdynamic -lSpectrum
VERSION = -std=c++11

HEADERS = src/*.h
OBJS = src/*.cpp

NAME = DRS_Plots

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	$(ROOTBUILD) $(VERSION) $(OBJS) -o $(NAME) $(ROOTLIBS)

debug: $(OBJS) $(HEADERS)
	$(ROOTBUILD) $(VERSION) $(OBJS) -o $(NAME) $(ROOTLIBS) -g

clean:
	rm $(NAME)

