TARGET = px86
SRCDIR = src
SRCS = $(wildcard $(SRCDIR)/*.c)
OUTDIR = build
OBJS = $(addprefix $(OUTDIR)/, $(notdir $(SRCS:.c=.o)))

CC = gcc
CFLAGS = -Wall

all : $(TARGET)

clean :
	@rm -rf build
	@rm $(TARGET)

$(OUTDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
