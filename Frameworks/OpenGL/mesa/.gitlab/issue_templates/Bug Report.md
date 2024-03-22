### Before submitting your bug report:
- Check if a new version of Mesa is available which might have fixed the problem.
- If you can, check if the latest development version (git main) works better.
- Check if your bug has already been reported here.
- For any logs, backtraces, etc - use [code blocks](https://docs.gitlab.com/ee/user/markdown.html#code-spans-and-blocks), GitLab removes line breaks without this.
   - Do not paste long logs directly into the description. Use https://gitlab.freedesktop.org/-/snippets/new, attachments, or a pastebin with a long expiration instead.
- As examples of good bug reports you may review one of these - #2598, #2615, #2608


Otherwise, please fill the requested information below.
And please remove anything that doesn't apply to keep things readable :)


The title should effectively distinguish this bug report from others and be specific to issue you encounter. When writing the title of the bug report, include a short description of the issue, the hardware/driver(s) affected and application(s) affected.


### System information

Please post `inxi -GSC -xx` output ([fenced with triple backticks](https://docs.gitlab.com/ee/user/markdown.html#code-spans-and-blocks)) OR fill information below manually


- OS: (`cat /etc/os-release | grep "NAME"`)
- GPU: (`lspci -nn | grep VGA` or `lshw -C display -numeric`)
- Kernel version: (run `uname -a`)
- Mesa version: (`glxinfo -B | grep "OpenGL version string"`)
- Xserver version (if applicable): (`sudo X -version`)
- Desktop manager and compositor:

#### If applicable
- DXVK version:
- Wine/Proton version:


### Describe the issue

Please describe what you are doing, what you expect and what you're
seeing instead.  How frequent is the issue? Is it a one time occurrence? Does it appear multiple times but randomly? Can you easily reproduce it?

"It doesn't work" usually is not a helpful description of an issue. 
The more detail about how things are going wrong, the better.


### Regression

Did it used to work? It can greatly help to know when the issue started.


### Log files as attachment
- Output of `dmesg`
- Backtrace
- Gpu hang details


### Screenshots/video files (if applicable)



### Any extra information would be greatly appreciated
