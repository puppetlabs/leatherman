# How to contribute

Third-party patches are essential for keeping leatherman great. We simply can't
access the huge number of platforms and myriad configurations for running
leatherman. We want to keep it as easy as possible to contribute changes that
get things working in your environment. There are a few guidelines that we
need contributors to follow so that we can have a chance of keeping on
top of things.

## Getting Started

* Make sure you have a [Jira account](https://tickets.puppetlabs.com).
* Make sure you have a [GitHub account](https://github.com/signup/free).
* Submit a Jira ticket for your issue if one does not already exist.
  * Clearly describe the issue including steps to reproduce when it is a bug.
  * Make sure you fill in the earliest version that you know has the issue.
  * A ticket is not necessary for trivial changes
* Fork the repository on GitHub.

## New Libraries

All new libraries should include a short section in the README describing how
to setup and use the library.

## Making Changes

* Create a topic branch from where you want to base your work.
  * This is usually the master branch.
  * Only target release branches if you are certain your fix must be on that
    branch.
  * If you use leatherman as part of the [puppet-agent](https://github.com/puppetlabs/puppet-agent),
    double-check the base branch to make sure your fix gets in the correct
    stream.

    | leatherman | puppet-agent |
    |------------|--------------|
    | 1.4.x      | 5.5.x        |
    | 1.5.x      | 6.0.x        |
    | 1.6.x      | 6.4.x        |
    | master     | master       |

    For example, if you use puppet5 you will want to base your work on top of
    the `1.4.x` branch, so your fix would end up in a subsequent release of
    puppet-agent `5.5.x`.

    Once merged, your work will be automatically promoted to the other release
    streams when our internal CI passes.
  * To quickly create a topic branch based on master, run `git checkout -b
    fix/master/my_contribution master`. Please avoid working directly on the
    `master` branch.
* Make commits of logical and atomic units.
* Check for unnecessary whitespace with `git diff --check` before committing.
* If you have python 2 in your path you can run `make cpplint` to ensure your
  code formatting is clean. The linter runs as part of Travis CI and could fail
  the CI build.
* If you have cppcheck in your path you can run `make cppcheck` to ensure your
  code passes static analysis. cppcheck runs as part of Travis CI and could
  fail the CI build.
* Make sure your commit messages are in the proper format.

````
    (LTH-1234) Make the example in CONTRIBUTING imperative and concrete

    Without this patch applied the example commit message in the CONTRIBUTING
    document is not a concrete example.  This is a problem because the
    contributor is left to imagine what the commit message should look like
    based on a description rather than an example.  This patch fixes the
    problem by making the example concrete and imperative.

    The first line is a real life imperative statement with a ticket number
    from our issue tracker. The body describes the behavior without the patch,
    why this is a problem, and how the patch fixes the problem when applied.
````

* Make sure you have added the necessary tests for your changes.
* Run _all_ the tests to assure nothing else was accidentally broken.

## Making Trivial Changes

For changes of a trivial nature, it is not always necessary to create a new
ticket in Jira. In this case, it is appropriate to start the first line of a
commit with one of  `(docs)`, `(maint)`, or `(packaging)` instead of a ticket
number.

If a Jira ticket exists for the documentation commit, you can include it
after the `(docs)` token.

```
    (docs)(DOCUMENT-000) Add docs commit example to CONTRIBUTING

    There is no example for contributing a documentation commit
    to the Leatherman repository. This is a problem because the contributor
    is left to assume how a commit of this nature may appear.

    The first line is a real-life imperative statement with '(docs)' in
    place of what would have been the LTH project ticket number in a
    non-documentation related commit. The body describes the nature of
    the new documentation or comments added.
```

For commits that address trivial repository maintenance tasks or packaging
issues, start the first line of the commit with `(maint)` or `(packaging)`,
respectively.

## Submitting Changes

* Sign the [Contributor License Agreement](https://cla.puppet.com)
* Push your changes to a topic branch in your fork of the repository.
* Submit a pull request to the repository in the puppetlabs organization.
* Update your ticket to mark that you have submitted code and are ready for it to be reviewed.
  * Include a link to the pull request in the ticket

# Additional Resources

* [More information on contributing](http://links.puppetlabs.com/contribute-to-puppet)
* [Bug tracker (Jira)](https://tickets.puppetlabs.com/browse/LTH)
* [Contributor License Agreement](http://links.puppetlabs.com/cla)
* [General GitHub documentation](http://help.github.com/)
* [GitHub pull request documentation](http://help.github.com/send-pull-requests/)
* #puppet-dev IRC channel on freenode.org
