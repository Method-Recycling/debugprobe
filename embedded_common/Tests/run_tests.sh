# Generate makefile
cmake .
echo 'makefile generated!'

# Build tests
make
echo 'tests built!'

# Run tests
if ./embedded_common_tests; then
    echo 'tests passed'
    exit 0
else
    echo 'tests failed'
    exit 1
fi

exit 1