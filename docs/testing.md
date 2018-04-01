# Testing

## Running Tests

Tests are run using CTest. To run tests, do the following in your build
directory:

```sh
$ make all test
```

To additionally see test coverage, run:

```sh
$ make all test coverage
```

## Writing Tests

This project uses the UnitTest++ testing framework. Please see the documentation
available [here](https://github.com/unittest-cpp/unittest-cpp/wiki).
