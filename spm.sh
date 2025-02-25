#!/bin/bash

usage() {
  echo "spm: usage: $0 get <library name>"
}

if [[ $# -eq 0 ]]; then
  usage
  exit 1
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    "get")
      shift
      if [[ -z $1 ]]; then
        usage
        exit 1
      fi
      LIB=$1
      shift
      ;;
    *)
      usage
      exit 1
      ;;
  esac
done

wget -q https://github.com/agayevhuseyn/seal-libs/raw/refs/heads/master/$LIB/$LIB.so

if [[ $? -eq 0 ]]; then
  echo "spm: '$LIB' library downloaded successfully"
else
  echo "spm: failed to get '$LIB' library"
fi
