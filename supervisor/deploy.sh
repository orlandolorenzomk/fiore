#!/bin/bash
REMOTE_USER="root"
REMOTE_HOST="192.168.1.139"
REMOTE_DIR="/fiore/supervisor"
LOCAL_DIR="$(pwd)"
PASSWORD="password"

sshpass -p "$PASSWORD" ssh "$REMOTE_USER@$REMOTE_HOST" "mkdir -p $REMOTE_DIR/state $REMOTE_DIR/logs"
sshpass -p "$PASSWORD" rsync -avz --delete \
    --exclude='state/' \
    --exclude='logs/' \
    --exclude='bin/' \
    --exclude='build/' \
    "$LOCAL_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/"
sshpass -p "$PASSWORD" ssh "$REMOTE_USER@$REMOTE_HOST" "cd $REMOTE_DIR && gmake clean && gmake && ./bin/supervisor $*"