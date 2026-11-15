#!/usr/bin/env bash

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
SCRIPT_NAME="elevator.8th"
SCRIPT_FILE="$SCRIPT_DIR/${SCRIPT_NAME}"
EIGHTH="$(command -v 8th)"
EIGHTH_OPTS="-q"

if [ ! -x "$EIGHTH" ]; then
	echo "8th Interpreter not found - cannot continue!"
	exit 200
fi

if [ ! -f "${SCRIPT_FILE}" ]; then
	echo "Script file ${SCRIPT_NAME} not found - cannot continue!"
	exit 201
fi

# echo "     EIGHTH = ${EIGHTH}"
# echo "EIGHTH_OPTS = ${EIGHTH_OPTS}"
# echo " SCRIPT_DIR = ${SCRIPT_DIR}"
# echo "SCRIPT_FILE = ${SCRIPT_FILE}"

exec ${EIGHTH} ${EIGHTH_OPTS} ${SCRIPT_FILE}
