#!/usr/bin/env cosh

echo "=== cos diagnostic (pid: $$) ==="

# filesystem
echo "--- vfs ---"
pwd
ls / && ls /bin && ls /usr/lib

# fs write round-trip: create, verify, check non-empty
mkdir /tmp
touch /tmp/diagtest
[ -f /tmp/diagtest ] && echo "touch: ok" || echo "touch: FAIL"
[ -s /tmp/diagtest ] || echo "note: diagtest is empty (expected)"
stat /tmp/diagtest

# builtins
echo "--- builtins ---"
true  && echo "true:  ok"
false || echo "false: ok"
[ -n "hello" ] && echo "test -n: ok"
[ -z "" ]      && echo "test -z: ok"
[ 2 -gt 1 ]    && echo "test -gt: ok"
[ "a" = "a" ]  && echo "test =: ok"

# chdir round-trip
cd /tmp && pwd && cd / && pwd && echo "cd: ok"

# eval
eval echo eval says hello && echo "eval: ok"

# $? propagation
true;  echo "after true:  $?"
false; echo "after false: $?"

echo "=== done ==="
