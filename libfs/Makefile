# For our makefile, we got most of the information from the slides. 
# Therefore, most of what is seen is similar to the Makefile in apps and similar to the slides from discussion. 
#I used my make file from Project 2 and altered it a bit to work for this project.
# Target library
target := libfs.a
objs:= disk.o fs.o 
# Flags
CC  := gcc
CFLAGS := -Wall -Wextra -Werror 
AR  := ar rcs 

# Debug Flag
ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

# Option for showcasing the commands 
ifneq ($(V),1)
Q = @
V = 0
endif

# To include dependencies  
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

# Making to compile the program itself 
all: $(target)

# Compiling the library
$(target): $(objs) 
	@echo "AR	$@"
	$(Q)$(AR) $@ $(objs)

# Generalized function for creating a object file 
%.o: %.c
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	@echo "clean"
	$(Q)rm -f $(target) $(objs) $(deps)

