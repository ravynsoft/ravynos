#!perl -w
# Don't use strict because this is for testing require

package test_require;

++$test_require::loaded;
