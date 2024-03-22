SkQP
====

`SkQP <https://skia.org/docs/dev/testing/skqp/>`__ stands for SKIA Quality
Program conformance tests.  Basically, it has sets of rendering tests and unit
tests to ensure that `SKIA <https://skia.org/>`__ is meeting its design specifications on a specific
device.

The rendering tests have support for GL, GLES and Vulkan backends and test some
rendering scenarios.
And the unit tests check the GPU behavior without rendering images, using any of the GL/GLES or Vulkan drivers.

SkQP reports
------------

SkQP generates reports after finishing its execution, and deqp-runner collects
them in the job artifacts results directory under the test name.  Click the
'Browse' button from a failing job to get to them.

SkQP failing tests
------------------

SkQP rendering tests will have a range of pixel values allowed for the driver's
rendering for a given test.  This can make the "expected" image in the result
output look rather strange, but you should be able to make sense of it knowing
that.

In SkQP itself, testcases can have increased failing pixel thresholds added to
them to keep CI green when the rendering is "correct" but out of normal range.
However, we don't support changing the thresholds in our testing.  Because any
driver rendering not meeting the normal thresholds will trigger Android CTS
failures, we treat them as failures and track them as expected failures the
```*-fails.txt`` file.`
