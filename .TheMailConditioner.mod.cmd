savedcmd_TheMailConditioner.mod := printf '%s\n'   TheMailConditioner.o | awk '!x[$$0]++ { print("./"$$0) }' > TheMailConditioner.mod
