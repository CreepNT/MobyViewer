# MobyViewer
This tool allows you to visualize Mobys in real-time from a chosen target platform (i.e. PS2 Emulator).

More target platforms and functionalities will be added in the future.

![Sample screenshot](https://raw.githubusercontent.com/CreepNT/MobyViewer/master/screenshot0.png)

# Usage Guide
Currently visible Mobys have their title bar colored in orange, to allow for easy distinguishing.<br>
Use the `Attach` button with PCSX2 launch to attach to the process, and all Mobys' data should appear.<br>
Use the buttons in the `Options and filters` box to adjust the shown Mobys.

You can add oClass lookup files along the main executable with the following format :
 * File name : `rcX.txt` where X is the game's number (1 for R&C1, 2 for R&C, etc)
 * Format : `0000=Moby Name`, where 0000 is the **HEXADECIMAL** of the oClass. oClass `FFFF` is reserved as an invalid oClass.
 * One line per Moby
 
This format is the same used in [Replanetizer](https://github.com/RatchetModding/replanetizer/blob/master/Replanetizer/ModelLists/ModelListRC1.txt).
  
# License
Everything in this repository is made avaliable under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause), unless specified otherwise.<br>
See `LICENSE.TXT` for more information.