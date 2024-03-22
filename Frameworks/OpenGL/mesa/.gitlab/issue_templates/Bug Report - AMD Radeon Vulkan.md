### Before submitting your bug report:
- Check if a new version of Mesa is available which might have fixed the problem.
- If you can, check if the latest development version (git main) works better.
- Check if your bug has already been reported here.
- For any logs, backtraces, etc - use [code blocks](https://docs.gitlab.com/ee/user/markdown.html#code-spans-and-blocks), GitLab removes line breaks without this.
   - Do not paste long logs directly into the description. Use https://gitlab.freedesktop.org/-/snippets/new, attachments, or a pastebin with a long expiration instead.
- As examples of good bug reports you may review one of these - #2598, #2615, #2608


Otherwise, fill the requested information below.
And please remove anything that doesn't apply to keep things readable :)


The title should effectively distinguish this bug report from others and be specific to issue you encounter. When writing the title of the bug report, include a short description of the issue, the hardware/driver(s) affected and application(s) affected.


### Description

Describe what you are doing, what you expect and what you're
seeing instead. How frequent is the issue? Is it a one time occurrence? Does it appear multiple times but randomly? Can you easily reproduce it?

"It doesn't work" usually is not a helpful description of an issue.
The more detail about how things are going wrong, the better.

### Screenshots/video files

For rendering errors, attach screenshots of the problem and (if possible) of how it should look. For freezes, it may be useful to provide a screenshot of the affected game scene. Prefer screenshots over videos.

### Log files (for system lockups / game freezes / crashes)

- Backtrace (for crashes)
- Output of `dmesg`
- Hang reports: Run with `RADV_DEBUG=hang` and attach the files created in `$HOME/radv_dumps_*/`

### Steps to reproduce

How can Mesa developers reproduce the issue? When reporting a game issue, start explaining from a fresh save file and don't assume prior knowledge of the game's story.

Example:

1. `Start new game and enter second mission (takes about 10 minutes)`
2. `Talk to the NPC called "Frank"`
3. `Observe flickering on Frank's body`

### System information

Please post `inxi -GSC -xx` output ([fenced with triple backticks](https://docs.gitlab.com/ee/user/markdown.html#code-spans-and-blocks)) OR fill information below manually


- OS: (`cat /etc/os-release | grep "NAME"`)
- GPU: (`lspci -nn | grep VGA` or `lshw -C display -numeric`)
- Kernel version: (`uname -a`)
- Mesa version: (`glxinfo -B | grep "OpenGL version string"`)
- Desktop environment: (`env | grep XDG_CURRENT_DESKTOP`)

#### If applicable
- Xserver version: (`sudo X -version`)
- DXVK version:
- Wine/Proton version:


### Regression

Did it used to work in a previous Mesa version? It can greatly help to know when the issue started.


### API captures (if applicable, optional)

Consider recording a [GFXReconstruct](https://github.com/LunarG/gfxreconstruct/blob/dev/USAGE_desktop_Vulkan.md) (preferred), [RenderDoc](https://renderdoc.org/), or [apitrace](https://github.com/apitrace/apitrace/blob/master/docs/USAGE.markdown) capture of the issue with the RADV driver active. This can tremendously help when debugging issues, but you're still encouraged to report issues if you can't provide a capture file.

### Further information (optional)

Does the issue reproduce with the LLVM backend (`RADV_DEBUG=llvm`) or on the AMDGPU-PRO drivers?

Does your environment set any of the variables `ACO_DEBUG`, `RADV_DEBUG`, and `RADV_PERFTEST`?
