TARGET = Xstatic

SRCS = src/main.c \
       src/protocol/framing.c \
       src/core/mesh_state.c \
       src/protocol/generated/meshtastic/mesh.pb.c \
       src/protocol/generated/meshtastic/channel.pb.c \
       src/protocol/generated/meshtastic/config.pb.c \
       src/protocol/generated/meshtastic/device_ui.pb.c \
       src/protocol/generated/meshtastic/module_config.pb.c \
       src/protocol/generated/meshtastic/portnums.pb.c \
       src/protocol/generated/meshtastic/telemetry.pb.c \
       src/protocol/generated/meshtastic/xmodem.pb.c \
       third_party/nanopb/pb_decode.c \
       third_party/nanopb/pb_encode.c \
       third_party/nanopb/pb_common.c

INCLUDES = -Isrc/platform -Isrc/protocol -Isrc/protocol/generated \
           -Isrc/core -Ithird_party/nanopb -I.

all:
	@case "`uname -s`" in \
		FreeBSD) PLATFORM_SRC=src/platform/platform_freebsd.c ;; \
		Linux)   PLATFORM_SRC=src/platform/platform_linux.c ;; \
		*) echo "OS non supporte: `uname -s`" >&2; exit 1 ;; \
	esac; \
	cc -o $(TARGET) $(SRCS) $$PLATFORM_SRC $(INCLUDES)

clean:
	rm -f $(TARGET)

.PHONY: all clean