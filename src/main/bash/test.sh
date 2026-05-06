#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0

echo "Compiler should accept..."
echo ""

for test in $(find src/main/bash/test/acceptance/stage-ii -name "*.clock" | sort); do
	cat "$test" | ".build/Flex-Bison-Compiler" >/dev/null 2>&1
	RESULT="$?"
	NAME="$(basename $test)"
	if [ "$RESULT" == "0" ]; then
		echo -e "    $NAME, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $NAME, ${RED}but it rejects${OFF} (status $RESULT)"
	fi
done
echo ""

echo "Compiler should reject..."
echo ""

for test in $(find src/main/bash/test/rejection/stage-ii -name "*.clock" | sort); do
	cat "$test" | ".build/Flex-Bison-Compiler" >/dev/null 2>&1
	RESULT="$?"
	NAME="$(basename $test)"
	if [ "$RESULT" != "0" ]; then
		echo -e "    $NAME, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $NAME, ${RED}but it accepts${OFF} (status $RESULT)"
	fi
done
echo ""

echo "All done."
exit $STATUS
