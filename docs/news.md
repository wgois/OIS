# News

- back to [index](http://wgois.github.io/OIS)

This page is a simple "static" blog-like thing just to post updates about where OIS's developement is going

## Log entries

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
 
 
