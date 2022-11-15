## vol2birdR 0.2.0
This version is the initial release of this package,
resubmitting after second review by Benjamin Altmann

* missing \value tags added for get_install_urls
* expanded the example for rsl2odim

We carefully reviewed the remaining \dontrun{} blocks in examples. These blocks are in place because these examples trigger or require large file downloads, which we therefore don't want to run in continuous integration tests. We have replaced them with less strict \donttest() blocks where possible, except for two instances where the user is required to adjust the paths in the examples.
