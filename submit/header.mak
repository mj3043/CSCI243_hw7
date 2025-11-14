# Note the addition of -Werror, which tells the compiler to treat
# all warning messages as fatal errors
CFLAGS=	-std=c99 -Wall -pedantic -Wextra -ggdb -Werror
CLIBFLAGS= -lm

