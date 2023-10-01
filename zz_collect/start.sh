#! /bin/sh

nohup ./bin/zz_collect > /dev/null 2>&1 &

echo "start zz_collect pid:$!"
echo "$!" > log/zz_collect.pid
