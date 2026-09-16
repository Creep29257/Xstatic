TARGET = Xstatic
DEBUG_TARGET = Xstatic-debug

COMMON_SRCS = src/protocol/framing.c \
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
       third_party/nanopb/pb_common.c \
       src/protocol/hw_model_name.c \
       src/core/device_config.c \
       src/protocol/config_enum_name.c

INCLUDES = -Isrc/platform -Isrc/protocol -Isrc/protocol/generated \
           -Isrc/core -Ithird_party/nanopb -I.

all: $(TARGET) $(DEBUG_TARGET)

$(TARGET):
	@case "`uname -s`" in \
		FreeBSD) PLATFORM_SRC=src/platform/platform_freebsd.c ;; \
		Linux)   PLATFORM_SRC=src/platform/platform_linux.c ;; \
		*) echo "OS non supporte: `uname -s`" >&2; exit 1 ;; \
	esac; \
	cc -o $(TARGET) src/main_interactive.c $(COMMON_SRCS) $$PLATFORM_SRC $(INCLUDES)

$(DEBUG_TARGET):
	@case "`uname -s`" in \
		FreeBSD) PLATFORM_SRC=src/platform/platform_freebsd.c ;; \
		Linux)   PLATFORM_SRC=src/platform/platform_linux.c ;; \
		*) echo "OS non supporte: `uname -s`" >&2; exit 1 ;; \
	esac; \
	cc -o $(DEBUG_TARGET) src/main.c $(COMMON_SRCS) $$PLATFORM_SRC $(INCLUDES)

clean:
	rm -f $(TARGET) $(DEBUG_TARGET)

.PHONY: all clean