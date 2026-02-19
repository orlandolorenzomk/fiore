#!/bin/sh

# VM connection settings
HOST="localhost"
PORT="2222"
USER="root"
PASS="password"
REMOTE_DIR="/fiore-ss/jpack"

SSH="sshpass -p $PASS ssh -p $PORT -o StrictHostKeyChecking=no"
SCP="sshpass -p $PASS scp -P $PORT -o StrictHostKeyChecking=no"

echo "==> Syncing files to $USER@$HOST:$REMOTE_DIR ..."
sshpass -p "$PASS" rsync -az --delete \
    -e "ssh -p $PORT -o StrictHostKeyChecking=no" \
    --exclude '.git' \
    --exclude 'bin/' \
    --exclude 'build/' \
    ./ $USER@$HOST:$REMOTE_DIR

echo "==> Building on VM ..."
$SSH $USER@$HOST "cd $REMOTE_DIR && make clean && make -j10"

echo "==> Running ./bin/jpack on VM ..."
$SSH -t $USER@$HOST "cd $REMOTE_DIR && ./bin/jpack" "$@"
