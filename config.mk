

WITH_DEBUG := no


ifeq ($(WITH_DEBUG),yes)
	CFLAGS := $(CFLAGS) -g
endif