sail_test(TARGET bugs                   SOURCES bugs.c                   LINK sail)
sail_test(TARGET io-produce-same-images SOURCES io-produce-same-images.c LINK sail sail-comparators)
