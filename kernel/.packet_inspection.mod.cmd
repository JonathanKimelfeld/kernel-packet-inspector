savedcmd_packet_inspection.mod := printf '%s\n'   packet_inspection.o | awk '!x[$$0]++ { print("./"$$0) }' > packet_inspection.mod
