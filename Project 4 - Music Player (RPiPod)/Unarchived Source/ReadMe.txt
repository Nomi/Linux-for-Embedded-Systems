I should have not made this use InitRAMFS for two reasons:

    1. Debugging is very hard because image needs to be compiled after every change and 
    then flashed to the RPi (requiring multiple reboots).

    2. If this is supposed to be a music player, we need to store new songs in it. However, 
    with InitRAMFS we're stuck with the same songs provided in the Overlay at the beginning.
    

    
As such, I should have made a rootfs partition (maybe using ext2/4, or something else) for this instead.