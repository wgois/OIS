# News

- back to [index](http://wgois.github.io/OIS)

This page is a simple "static" blog-like thing just to post updates about where OIS's developement is going

## Log entries

### 12/01/2018

#### Prepraing for OIS 1.5 release

This is just a short post to notify that the release 1.5 is now in preparation!

The one single feature that needed merging is now merged (added modifier bitflags for capslock and numlock state. These are toggeled at each keydown event on their respective key).

I'm currently running a little pass of static analisys and fixes. build system and API doesn't have any real changes between 1.4 and 1.5, it's just that what point in time actually is 1.4 is a bit nebulous. I tried to tag the correct "commit", but between what was shipped in the "Ogre dependencies package" for a while, what was maintained in an independent fork (that was now merged in this repository, it's kind of a mess).

1.5 is to get a nice "milestone" in time after a few years of accumulating dust. I was meaning to do this release in the summer, but we are december frist, so I'm a bit late... This is just equivalent to the 1.4.1 fersion mentioned last april... :smile: 

Anyway, the documentation on github pages is going to be updated soon, and I'll do my best to notify package mantainers, members of the OGRE forums and any other places here and there...

Stay tuned!

### 04/26/2018

Hi, Ybalrid here, 

I recently (about a month ago) got access to the OIS repository to help maintain the library.

Since that time, I managed to go through the few oppened pull request that accumulated on GitHub over the last couple of years. Most of them were not mergable as-is. Some of them got merged-in, or applied by hand. Other where closed because the problem they where fixing was already adressed. And Issues where oppened to address the remaining problems.

I am currently (and I'm almost done) with this. Progress can be followed on this kan-ban style board here https://github.com/wgois/OIS/projects/1

Once I'm done with that, and we're in a state where our CI tells that everything is green, I'm going to tag a 1.4.1 release, then try to get a few package maintainers from Linux distributions to update their packages, upload windows binaries, and probably update the guys from the OgreDeps project to check out the new stuff.

Basically, the changes as of now can be sumarized like this : 
 - some bugs were corrected
 - build system swithced to CMake
 - A series of patches adding Cocoa support was added
 
 I'll do what's possible to keep that library alive. I'll post here news about where I thik we should go to modernize and cleanup the code, add some missing backed (Xinput!) and maybe some other features.
 
 
