# WFA

A short, early prototype of a game made in C with SDL. Similar to a side scroller, the player must navigate an unsuitably large yet powerful vessel through a dangerous asteroid field to explore the unknown.

The screenshot directory has a screenshot of the code in action.

This was developed using the SDL libraries (found here: https://www.libsdl.org/index.php) with some help and guidance from some Lazy Foo' (http://lazyfoo.net/tutorials/SDL/index.php).  The assets are a mix of self-made (if it looks like it was scribbled in MSPaint, that was me), requisitioned (the main ship silhouette is a based on the Archon-class from Star Trek Online), and volunteered material by a friend (Koppenflak who rendered everything that actually looks good). I merely claim the code.


This has only been developed with Windows in mind. Sorry, Linux guys, maybe soon. To compile on your own, you will require the SDL libraries. To run, simply extract all files from the archive and run WFA.EXE.  All needed DLLs have been included, thus it should be a complete package.


Controls:

WASD - Movement/Turning

Space - Pause/Unpause

LMB - Select primary target

1 - Fire Phasers at primary target

2 - Fire Photon Torpedo Spread forward

3 - Fire Quantum Torpedo Salvo forward

4 - Phase Cloak (allows player to ignore collision briefly, but with a long cooldown)



Shields will regenerate over time, however once shields are down the next hit is lethal. Once down, they will restore to 25% after 10 seconds. Try not to die, I like this ship.
