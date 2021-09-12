# Migration for background images

How the background image is drawn has been changed since version 0.17.
Intuitively, the background image is now drawn above the background color instead of below it.
Technically, the background image is no longer blended with the background color.

Any background image can be used but, of course, it should be chosen so that the terminal text can be easily read on it.
Since an image may not be totally dark or light, you might want to use a translucent image as the background.
As a result, the background image is mixed with the background color to improve readability.
Opaque images can also be converted to translucent ones with a few steps.

A common usage is an effect similiar to previous qtermwidget versions or other terminal emulators.
To achieve that, you can convert the background image to a translucent one with the transparency level matching the original terminal transparency.
For example, if the original terminal transparency of qtermwidget was 25% (or 75% in some other terminal emulators), a converted image with transparency 25% will work as usual.
The conversion can be done via ImageMagick, GraphicsMagick, GIMP or Krita.
Here is an example command using ImageMagick:

    $ convert original_image.jpg -matte -channel A +level 0,25% +channel translucent_image.png

You may also want to change the terminal transparency to 0% if you do not want to see another window or the desktop below the terminal.
