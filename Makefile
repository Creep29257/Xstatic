TARGET = test

SRCS = src/main.c \
       src/platform/platform_freebsd.c \
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

$(TARGET): $(SRCS)
	cc -o $(TARGET) $(SRCS) $(INCLUDES)

clean:
	rm -f $(TARGET)

.PHONY: clean
