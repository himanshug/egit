for i in `ls src/test/test_* | grep -v '\.c' `
do
    $i
done
