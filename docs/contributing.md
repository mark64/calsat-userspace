# Contributing

## Code Organization

Please follow the organization laid out in
[project-organization.md](docs/project-organization.md).

## Branches

Please make a new topic branch (such as "cmd-srv") when adding code to this
project. Do not commit directly to master.

## Documentation

All header files in the [include](include/) subdirectory represent public APIs.
As such, they should be extremely well documented. Source files and headers in
[src](src/) do not need to be documented, though if you use particularly obtuse
constructs you should clarify their purpose.

## Sanity Checks

Before submitting a pull request, make sure your commit passes all the tests. To
run tests, see [testing.md](docs/testing.md).

## Test Coverage

This project has a 100% test coverage goal. Obviously, that is difficult given
that some code requires special hardware to test, but if your code does not have
100% test coverage, there must be a very good reason. Otherwise, your pull
request will not be accepted.

## Pull Request Process

Once you've completed your changes, passed all the tests, and pushed to your
topic branch on GitHub, you can submit a pull request to master. Pull requests
require various automated tests to pass (such as Travis-CI) as well as one other
person to approve your code. If your work contributes to the project, is
well-designed, and follows all the requirements outlined above, your request
will be accepted.
