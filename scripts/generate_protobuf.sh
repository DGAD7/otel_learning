#!/bin/bash

set -e
set -x

SCRIPT_PATH=$(dirname "$0")
ROOT_PATH=$(realpath "$SCRIPT_PATH/..")

PROTO_FILES="proto/*.proto"

cd "$ROOT_PATH"

CPP_OUT="$ROOT_PATH"/control/grpc/generated
mkdir -p "$CPP_OUT" ui/grpc/generated

PROTOC=$(which protoc)
# Generate for controller
$PROTOC -I=./proto \
	--cpp_out=$CPP_OUT \
	--grpc_out=$CPP_OUT --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
	$PROTO_FILES

# Generate for UI
$PROTOC -I=./proto \
	--go_out=. \
	--go-grpc_out=. \
	$PROTO_FILES
